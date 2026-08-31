//
// Created by jaket on 25/03/2026.
//

#include "expressions/DotOperator.h"

#include "expressions/interface.h"
#include "expressions/accessors/FieldAccessor.h"
#include "expressions/accessors/PortAccessor.h"
#include "expressions/accessors/SwizzleAccessor.h"
#include "runtime/Type.h"
#include "runtime/variables/Variable.h"
#include "utils/swizzle_utils.h"

namespace mxslc::expressions
{
    DotOperator::DotOperator(ExprPtr value_expr, Token property) : Expression{std::move(property)}, value_expr_{std::move(value_expr)}
    {

    }

    ExprPtr DotOperator::monomorphize(const TypePtr& template_type) const
    {
        ExprPtr expr = value_expr_->monomorphize(template_type);
        return create_expression<DotOperator>(std::move(expr), token_);
    }

    void DotOperator::init_subexpressions(const vector<TypePtr>& types)
    {
        value_expr_->init();
    }

    void DotOperator::init_impl(const vector<TypePtr>& types)
    {
        if (value_expr_->type()->is_primitive())
        {
            if (swizzle_utils::is_valid_swizzle(value_expr_->type(), token_.lexeme()))
            {
                accessor_ = create_accessor<SwizzleAccessor>(value_expr_, token_.lexeme());
            }
            else
            {
                VarPtr value = value_expr_->evaluate();
                accessor_ = create_accessor<PortAccessor>(std::move(value), token_.lexeme());
            }
        }
        else
        {
            accessor_ = create_accessor<FieldAccessor>(types, value_expr_, token_.lexeme());
        }
    }

    TypePtr DotOperator::type_impl() const
    {
        return accessor_->type();
    }

    VarPtr DotOperator::evaluate_impl() const
    {
        return accessor_->evaluate();
    }

    string DotOperator::to_string() const
    {
        return value_expr_->to_string() + "." + token_.lexeme();
    }
}
