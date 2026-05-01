//
// Created by jaket on 17/04/2026.
//

#include "Runtime.h"
#include "Scope.h"

std::unique_ptr<Runtime> Runtime::instance_ = nullptr;

Runtime::Runtime()
    : Runtime{std::make_unique<Scope>(), MtlXSerializer{}}
{

}
Runtime::Runtime(ScopePtr scope, MtlXSerializer serializer)
    : scope_{std::move(scope)}, serializer_{std::move(serializer)}
{

}

Scope& Runtime::scope()
{
    return *scope_;
}

MtlXSerializer& Runtime::serializer()
{
    return serializer_;
}

void Runtime::enter_scope(bool is_inline)
{
    scope_ = std::make_unique<Scope>(std::move(scope_), is_inline);
}

void Runtime::enter_inline_scope()
{
    enter_scope(true);
}

void Runtime::exit_scope()
{
    scope_ = scope_->exit();
}

Runtime& Runtime::create()
{
    instance_ = std::make_unique<Runtime>();
    return *instance_;
}

Runtime& Runtime::get()
{
    if (instance_ == nullptr)
        instance_ = std::make_unique<Runtime>();
    return *instance_;
}
