//
// Created by jaket on 28/05/2026.
//

#include "expressions/accessors/ComponentAccessor.h"

#include "runtime/ComponentVariable.h"

namespace mxslc::expressions
{
    ComponentAccessor::ComponentAccessor(ExprPtr value_expr, ExprPtr index_expr)
    {
        component_var_ = std::make_shared<ComponentVariable>(std::move(value_expr), std::move(index_expr));
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
