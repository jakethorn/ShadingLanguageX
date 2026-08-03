//
// Created by jaket on 28/05/2026.
//

#include "expressions/accessors/ComponentAccessor.h"

#include "expressions/Expression.h"
#include "runtime/variables/ComponentVariable.h"

namespace mxslc::expressions
{
    ComponentAccessor::ComponentAccessor(const ExprPtr& value_expr, const ExprPtr& index_expr)
    {
        VarPtr value = value_expr->evaluate();
        VarPtr index = index_expr->evaluate();
        component_var_ = std::make_shared<ComponentVariable>(std::move(value), std::move(index));
    }

    TypePtr ComponentAccessor::type() const
    {
        return component_var_->type();
    }

    VarPtr ComponentAccessor::evaluate() const
    {
        return component_var_;
    }
}
