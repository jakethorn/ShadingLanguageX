//
// Created by jaket on 28/05/2026.
//

#include "expressions/accessors/ComponentAccessor.h"

#include "expressions/FunctionCall.h"
#include "expressions/interface.h"
#include "runtime/utils/invoke.h"

namespace mxslc::expressions
{
    ComponentAccessor::ComponentAccessor(ExprPtr value_expr, ExprPtr index_expr)
    {
        func_call_ = create_expression<FunctionCall>("extract", ArgumentList{std::move(value_expr), std::move(index_expr)});
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
