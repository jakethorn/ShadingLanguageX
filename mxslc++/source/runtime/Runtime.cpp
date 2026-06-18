//
// Created by jaket on 17/04/2026.
//

#include "Runtime.h"

#include "Scope.h"
#include "mtlx/load_mtlx.h"
#include "CompileError.h"
#include "Variable.h"
#include "utils/io_utils.h"

std::unique_ptr<Runtime> Runtime::instance_ = nullptr;

Runtime::Runtime(const CompileOptions& opts) : Runtime{opts, std::make_unique<Scope>(), MtlXSerializer{}}
{

}

Runtime::Runtime(const CompileOptions& opts, ScopePtr scope, MtlXSerializer serializer) : opts_{opts}, scope_{std::move(scope)}, serializer_{std::move(serializer)}
{
    scope_->set_graph(serializer_.document(), nullptr);
}

namespace
{
    vector<fs::path> get_include_directories(const optional<fs::path>& src_path)
    {
        vector<fs::path> dirs;
        if (src_path)
            dirs.push_back(src_path->parent_path());
        dirs.push_back(fs::current_path());
        dirs.push_back(get_python_module_dir());
        dirs.push_back(get_executable_dir());
        return dirs;
    }
}

Runtime& Runtime::create(const optional<fs::path>& src_path, const CompileOptions& opts)
{
    instance_ = std::make_unique<Runtime>(opts);
    instance_->include_dirs_ = get_include_directories(src_path);
    instance_->serializer_.set_reduce_graph(opts.reduce_graph);
    instance_->load_materialx_library(opts.version);
    return *instance_;
}

Runtime& Runtime::get()
{
    if (instance_ == nullptr)
        throw CompileError{"Runtime not created"s};
    return *instance_;
}

VarPtr Runtime::global(const string& name) const
{
    for (const mxslc::Variable& global : opts_.globals)
    {
        if (global.name() == name)
        {
            used_globals.push_back(name);
            return Variable::create(global);
        }
    }
    if (opts_.error_on_missing_globals)
        throw CompileError{"Missing global variable: " + name};
    return nullptr;
}

void Runtime::load_materialx_library(const string& version)
{
    mtlx_lib_ = get_materialx_library(version);
    load_library(mtlx_lib_);
}

Scope& Runtime::scope()
{
    return *scope_;
}

void Runtime::enter_scope(string name)
{
    scope_ = std::make_unique<Scope>(std::move(name), std::move(scope_));
}

void Runtime::exit_scope()
{
    scope_ = scope_->exit();
}

MtlXSerializer& Runtime::serializer()
{
    return serializer_;
}

void Runtime::destroy() const
{
    if (not opts_.error_on_unused_globals)
        return;

    for (const mxslc::Variable& global : opts_.globals)
    {
        if (not contains(used_globals, global.name()))
            throw CompileError{"Unused global variable: " + global.name()};
    }
}
