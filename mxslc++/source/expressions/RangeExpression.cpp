//
// Created by jaket on 15/06/2026.
//

#include "RangeExpression.h"

#include "runtime/Type.h"

RangeExpression::RangeExpression(ExprPtr lower_expr, ExprPtr upper_expr)
    : RangeExpression{std::move(lower_expr), nullptr, std::move(upper_expr), Token{}}
{

}

RangeExpression::RangeExpression(ExprPtr lower_expr, ExprPtr upper_expr, Token token)
    : RangeExpression{std::move(lower_expr), nullptr, std::move(upper_expr), std::move(token)}
{

}

RangeExpression::RangeExpression(ExprPtr lower_expr, ExprPtr step_expr, ExprPtr upper_expr)
    : RangeExpression{std::move(lower_expr), std::move(step_expr), std::move(upper_expr), Token{}}
{

}

RangeExpression::RangeExpression(ExprPtr lower_expr, ExprPtr step_expr, ExprPtr upper_expr, Token token)
    : Expression{std::move(token)}, lower_expr_{std::move(lower_expr)}, step_expr_{std::move(step_expr)}, upper_expr_{std::move(upper_expr)}
{

}

ExprPtr RangeExpression::instantiate_template_types(const TypePtr& template_type) const
{
    ExprPtr lower_expr = lower_expr_->instantiate_template_types(template_type);
    ExprPtr step_expr = step_expr_ ? step_expr_->instantiate_template_types(template_type) : nullptr;
    ExprPtr upper_expr = upper_expr_->instantiate_template_types(template_type);
    return std::make_unique<RangeExpression>(std::move(lower_expr), std::move(step_expr), std::move(upper_expr), token_);
}

void RangeExpression::init_subexpressions(const vector<TypePtr>& types)
{
    if (subexpr_type_ and not (subexpr_type_->is_auto() or subexpr_type_->is<float>() or subexpr_type_->is<int>()))
        throw CompileError{"Invalid range expression type: " + subexpr_type_->name()};

    const vector expr_types = subexpr_type_ ? vector{subexpr_type_} : vector{Type::Int, Type::Float};
    lower_expr_->init(expr_types);
    if (step_expr_)
        step_expr_->init(expr_types);
    upper_expr_->init(expr_types);
}

void RangeExpression::init_impl(const vector<TypePtr>& types)
{
    if (lower_expr_->type()->is<float>() and (step_expr_ == nullptr or step_expr_->type()->is<float>()) and upper_expr_->type()->is<float>())
    {
        init_as<float>();
    }
    else
    {
        init_as<int>();
    }
}

TypePtr RangeExpression::type_impl() const
{
    return range_->type();
}

VarPtr RangeExpression::evaluate_impl() const
{
    return range_;
}
