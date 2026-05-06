//
// Created by jaket on 05/05/2026.
//

#include "IncrementExpression.h"

#include "FunctionCall.h"
#include "runtime/Variable.h"

IncrementExpression::IncrementExpression(ExprPtr expr, Token op, const bool prefix)
    : Expression{std::move(op)}, expr_{std::move(expr)}, prefix_{prefix}
{
    increment_ = token_ == TokenType::Increment;
}

ExprPtr IncrementExpression::instantiate_template_types(const TypePtr& template_type) const
{
    ExprPtr expr = expr_->instantiate_template_types(template_type);
    return std::make_unique<IncrementExpression>(std::move(expr), token_, prefix_);
}

void IncrementExpression::init_subexpressions(const vector<TypePtr>& types)
{
    expr_->init(types);
}

TypePtr IncrementExpression::type_impl() const
{
    return expr_->type();
}

VarPtr IncrementExpression::evaluate_impl() const
{
    const VarPtr var = expr_->evaluate();
    VarPtr pre_var = Variable::create(var);

    ArgumentList args{expr_};
    const ExprPtr func_call = std::make_shared<FunctionCall>(increment_ ? "__inc__"s : "__dec__"s, std::move(args));

    func_call->init(expr_->type());
    VarPtr inc_var = func_call->evaluate();
    var->copy_value(inc_var);

    if (prefix_)
        return inc_var;
    else
        return pre_var;
}
