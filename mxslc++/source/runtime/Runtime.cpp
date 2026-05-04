//
// Created by jaket on 17/04/2026.
//

#include "Runtime.h"
#include "Scope.h"
#include "mtlx/load_mtlx.h"

std::unique_ptr<Runtime> Runtime::instance_ = nullptr;

Runtime::Runtime() : Runtime{std::make_unique<Scope>(), MtlXSerializer{}}
{

}
Runtime::Runtime(ScopePtr scope, MtlXSerializer serializer) : scope_{std::move(scope)}, serializer_{std::move(serializer)}
{
    scope_->set_graph(serializer_.document(), nullptr);
}

Scope& Runtime::scope()
{
    return *scope_;
}

MtlXSerializer& Runtime::serializer()
{
    return serializer_;
}

void Runtime::enter_scope()
{
    scope_ = std::make_unique<Scope>(std::move(scope_));
}

void Runtime::exit_scope()
{
    scope_ = scope_->exit();
}

Runtime& Runtime::create(const string& version)
{
    instance_ = std::make_unique<Runtime>();
    instance_->load_materialx_library(version);
    return *instance_;
}

Runtime& Runtime::get()
{
    if (instance_ == nullptr)
        throw CompileError{"Runtime not created"s};
    return *instance_;
}

void Runtime::load_materialx_library(const string& version)
{
    mtlx_lib_ = get_materialx_library(version);
    load_library(mtlx_lib_);
}
