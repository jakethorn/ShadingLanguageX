//
// Created by jaket on 17/04/2026.
//

#include "runtime/Runtime.h"

#include "CompileOptions.h"
#include "runtime/Scope.h"
#include "runtime/variables/Variable.h"
#include "utils/load_mtlx.h"
#include "utils/io_utils.h"
#include "errors/CompileError.h"
#include "runtime/interface.h"
#include "utils/Logger.h"

namespace mxslc::runtime
{
    using container_utils::contains;

    unique_ptr<Runtime> Runtime::instance_ = nullptr;

    Runtime::Runtime(CompileOptions opts) : opts_{std::move(opts)}
    {
        scope_ = create_scope("mtlx_stdlib");
        scope_->set_graph(serializer_.document(), nullptr);
        serializer_.set_version(opts_.version);
        serializer_.set_reduce_graph(opts_.reduce_graph);
        serializer_.set_single_use_as_nodegraph(opts_.single_use_as_nodegraph);
    }

    Runtime& Runtime::create(CompileOptions opts)
    {
        instance_ = std::make_unique<Runtime>(std::move(opts));
        instance_->load_libraries();
        instance_->load_materialx_library();
        return *instance_;
    }

    Runtime& Runtime::get()
    {
        if (instance_ == nullptr)
            throw CompileError{"Runtime not created"};
        return *instance_;
    }

    VarPtr Runtime::global(const string& name) const
    {
        if (opts_.has_global(name))
        {
            used_globals.push_back(name);
            return opts_.get_global(name)->copy();
        }
        if (opts_.error_on_missing_globals)
            throw CompileError{"Missing global variable: " + name};
        return nullptr;
    }

    void Runtime::load_libraries()
    {
        for (const fs::path& path : opts_.libraries)
        {
            io_utils::search(opts_.search_directories(), path, [](const fs::path& found_path) {
                load_library(found_path);
            });
        }
    }

    void Runtime::load_materialx_library()
    {
        mtlx_lib_ = get_materialx_library(opts_.version, opts_.search_directories());
        load_library(mtlx_lib_);
        Logger::debug("Loaded MaterialX version " + opts_.version + " libraries.");
    }

    Scope& Runtime::scope()
    {
        return *scope_;
    }

    void Runtime::enter_scope(string name)
    {
        scope_ = create_scope(std::move(name), std::move(scope_));
    }

    void Runtime::exit_scope()
    {
        scope_ = scope_->exit();
    }

    Serializer& Runtime::serializer()
    {
        return serializer_;
    }

    void Runtime::destroy() const
    {
        if (not opts_.error_on_unused_globals)
            return;

        for (const auto& [name, value] : opts_.globals())
        {
            if (not contains(used_globals, name))
                throw CompileError{"Unused global variable: " + name};
        }
    }
}
