//
// Created by jaket on 28/05/2026.
//

#include "expressions/RuntimeExpression.h"

#include "expressions/interface.h"
#include "runtime/interface.h"
#include "runtime/variables/Variable.h"

namespace mxslc::expressions
{
    RuntimeExpression::RuntimeExpression(Primitive value) : value_{create_variable(std::move(value))}
    {

    }

    RuntimeExpression::RuntimeExpression(ValuePtr value) : value_{create_variable(std::move(value))}
    {

    }

    RuntimeExpression::RuntimeExpression(VarPtr value) : value_{std::move(value)}
    {

    }

    ExprPtr RuntimeExpression::monomorphize(const TypePtr& template_type) const
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

    string RuntimeExpression::to_string() const
    {
        return value_->to_string();
    }
}
