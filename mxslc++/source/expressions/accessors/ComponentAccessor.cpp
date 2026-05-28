//
// Created by jaket on 28/05/2026.
//

#include "ComponentAccessor.h"

#include "expressions/FunctionCall.h"
#include "runtime/Runtime.h"
#include "runtime/Type.h"

ComponentAccessor::ComponentAccessor(ExprPtr value_expr, ExprPtr index_expr)
{
    func_call_ = std::make_shared<FunctionCall>("extract"s, ArgumentList{std::move(value_expr), std::move(index_expr)});
    func_call_->init();
}

TypePtr ComponentAccessor::type() const
{
    return func_call_->type();
}

VarPtr ComponentAccessor::evaluate() const
{
    return func_call_->evaluate();
}
