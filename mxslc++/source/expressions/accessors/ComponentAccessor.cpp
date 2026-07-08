//
// Created by jaket on 28/05/2026.
//

#include "expressions/accessors/ComponentAccessor.h"

#include "expressions/FunctionCall.h"
#include "runtime/utils/invoke_utils.h"

namespace mxslc::expressions
{
    ComponentAccessor::ComponentAccessor(ExprPtr value_expr, ExprPtr index_expr)
    {
        func_call_ = invoke_utils::function_call("extract", ArgumentList{std::move(value_expr), std::move(index_expr)});
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
}
