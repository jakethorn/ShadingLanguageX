//
// Created by jaket on 28/05/2026.
//

#include "expressions/RuntimeExpression.h"

#include "expressions/interface.h"
#include "runtime/Variable.h"

namespace mxslc::expressions
{
    RuntimeExpression::RuntimeExpression(VarPtr value) : value_{std::move(value)}
    {

    }

    ExprPtr RuntimeExpression::instantiate_template_types(const TypePtr& template_type) const
    {
        return create_expression<RuntimeExpression>(value_);
    }

    TypePtr RuntimeExpression::type_impl() const
    {
        return value_->type();
    }

    VarPtr RuntimeExpression::evaluate_impl() const
    {
        return value_;
    }
}
