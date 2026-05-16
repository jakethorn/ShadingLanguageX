//
// Created by jaket on 05/05/2026.
//

#include "IncrementExpression.h"

#include "FunctionCall.h"
#include "runtime/Variable.h"

IncrementExpression::IncrementExpression(ExprPtr lhs_expr, Token op, const bool prefix)
    : Expression{std::move(op)}, lhs_expr_{std::move(lhs_expr)}, prefix_{prefix}
{
    increment_ = token_ == TokenType::Increment;
}

ExprPtr IncrementExpression::instantiate_template_types(const TypePtr& template_type) const
{
    ExprPtr lhs_expr = lhs_expr_->instantiate_template_types(template_type);
    return std::make_unique<IncrementExpression>(std::move(lhs_expr), token_, prefix_);
}

void IncrementExpression::init_subexpressions(const vector<TypePtr>& types)
{
    lhs_expr_->init(types);
}

TypePtr IncrementExpression::type_impl() const
{
    return lhs_expr_->type();
}

VarPtr IncrementExpression::evaluate_impl() const
{
    const VarPtr lhs = lhs_expr_->evaluate();
    VarPtr original_lhs = lhs->copy();

    ArgumentList args{lhs_expr_};
    const ExprPtr func_call = std::make_shared<FunctionCall>(increment_ ? "__inc__"s : "__dec__"s, std::move(args));

    func_call->init(lhs_expr_->type());
    VarPtr incremented_lhs = func_call->evaluate();
    lhs->copy(incremented_lhs);

    if (prefix_)
        return incremented_lhs;
    else
        return original_lhs;
}
