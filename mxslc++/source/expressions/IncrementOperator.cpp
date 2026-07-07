//
// Created by jaket on 05/05/2026.
//

#include "expressions/IncrementOperator.h"

#include "expressions/FunctionCall.h"
#include "runtime/RuntimeUtils.h"
#include "runtime/Variable.h"

IncrementOperator::IncrementOperator(ExprPtr lhs_expr, Token op, const bool prefix)
    : Expression{std::move(op)}, lhs_expr_{std::move(lhs_expr)}, prefix_{prefix}
{
    increment_ = token_ == TokenType::Increment;
}

ExprPtr IncrementOperator::instantiate_template_types(const TypePtr& template_type) const
{
    ExprPtr lhs_expr = lhs_expr_->instantiate_template_types(template_type);
    return std::make_unique<IncrementOperator>(std::move(lhs_expr), token_, prefix_);
}

void IncrementOperator::init_subexpressions(const vector<TypePtr>& types)
{
    lhs_expr_->init(types);
}

TypePtr IncrementOperator::type_impl() const
{
    return lhs_expr_->type();
}

VarPtr IncrementOperator::evaluate_impl() const
{
    const VarPtr lhs = lhs_expr_->evaluate();
    VarPtr original_lhs = lhs->copy();

    string dunder_name = increment_ ? "__inc__" : "__dec__";
    const ExprPtr func_call = RuntimeUtils::function_call(std::move(dunder_name), ArgumentList{lhs_expr_});

    func_call->init(lhs_expr_->type());
    VarPtr incremented_lhs = func_call->evaluate();
    lhs->copy(incremented_lhs);

    if (prefix_)
        return incremented_lhs;
    else
        return original_lhs;
}
