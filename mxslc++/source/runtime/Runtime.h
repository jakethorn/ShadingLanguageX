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
    Runtime() : Runtime{std::make_unique<Scope>(), std::make_unique<MtlXSerializer>()} { }
    Runtime(ScopePtr scope, SerializerPtr serializer) : scope_{std::move(scope)}, serializer_{std::move(serializer)} { }

    [[nodiscard]] Scope& scope() const { return *scope_; }
    [[nodiscard]] const Serializer& serializer() const { return *serializer_; }

    void enter_scope()
    {
        scope_ = std::make_unique<Scope>(std::move(scope_));
    }

    void exit_scope()
    {
        scope_ = scope_->parent();
    }

private:
    ScopePtr scope_;
    SerializerPtr serializer_;
};

#endif //FENNEC_RUNTIME_H
