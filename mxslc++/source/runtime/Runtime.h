//
// Created by jaket on 02/01/2026.
//

#ifndef FENNEC_RUNTIME_H
#define FENNEC_RUNTIME_H

#include "utils/common.h"
#include "Scope.h"
#include "mtlx/MtlXSerializer.h"

class Runtime
{
public:
    Runtime() : Runtime{std::make_unique<Scope>(), MtlXSerializer{}} { }
    Runtime(ScopePtr scope, MtlXSerializer serializer) : scope_{std::move(scope)}, serializer_{std::move(serializer)} { }

    [[nodiscard]] Scope& scope() const { return *scope_; }
    [[nodiscard]] const MtlXSerializer& serializer() const { return serializer_; }

    void enter_scope(bool is_inline = false) const
    {
        scope_ = std::make_unique<Scope>(std::move(scope_), is_inline);
    }

    void enter_inline_scope() const
    {
        enter_scope(true);
    }

    void exit_scope() const
    {
        scope_ = scope_->exit();
    }

private:
    mutable ScopePtr scope_;
    MtlXSerializer serializer_;
};

#endif //FENNEC_RUNTIME_H
