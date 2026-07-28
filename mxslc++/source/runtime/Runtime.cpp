//
// Created by jaket on 17/04/2026.
//

#include "runtime/Runtime.h"

#include "CompileOptions.h"
#include "runtime/Scope.h"
#include "runtime/Variable.h"
#include "utils/load_mtlx.h"
#include "utils/io_utils.h"
#include "errors/CompileError.h"
#include "runtime/interface.h"

namespace mxslc::runtime
{
    using container_utils::contains;

    unique_ptr<Runtime> Runtime::instance_ = nullptr;

    Runtime::Runtime(CompileOptions opts) : Runtime{std::move(opts), create_scope(), Serializer{}}
    {

    }

    Runtime::Runtime(CompileOptions opts, ScopePtr scope, Serializer serializer) : opts_{std::move(opts)}, scope_{std::move(scope)}, serializer_{std::move(serializer)}
    {
        scope_->set_graph(serializer_.document(), nullptr);
    }

    Runtime& Runtime::create(CompileOptions opts)
    {
        instance_ = std::make_unique<Runtime>(std::move(opts));
        instance_->serializer_.set_reduce_graph(opts.reduce_graph);
        instance_->load_materialx_library(opts.version);
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
        for (const auto& [global_name, var] : opts_.globals)
        {
            if (global_name == name)
            {
                used_globals.push_back(name);
                return var->copy();
            }
        }
        if (opts_.error_on_missing_globals)
            throw CompileError{"Missing global variable: " + name};
        return nullptr;
    }

    void Runtime::load_materialx_library(const string& version)
    {
        mtlx_lib_ = get_materialx_library(version, include_directories());
        load_library(mtlx_lib_);
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

        for (const auto& [global_name, var] : opts_.globals)
        {
            if (not contains(used_globals, global_name))
                throw CompileError{"Unused global variable: " + global_name};
        }
    }
}
