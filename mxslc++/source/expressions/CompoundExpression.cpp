//
// Created by jaket on 15/05/2026.
//

#include "CompoundExpression.h"

#include "FunctionCall.h"
#include "runtime/ArgumentList.h"
#include "runtime/Variable.h"

CompoundExpression::CompoundExpression(ExprPtr lhs_expr, Token op, ExprPtr rhs_expr)
    : Expression{std::move(op)}, lhs_expr_{std::move(lhs_expr)}, rhs_expr_{std::move(rhs_expr)}
{
    static const unordered_map<TokenType, string> op_names {
            {"+="s, "__add__"},
            {"-="s, "__sub__"},
            {"*="s, "__mul__"},
            {"/="s, "__div__"},
            {"%="s, "__mod__"},
            {"^="s, "__pow__"},
            {"&="s, "__and__"},
            {"|="s, "__or__"},
        };

    string dunder_name = op_names.at(token_.type());
    ArgumentList args{lhs_expr_, rhs_expr_};
    func_call_ = std::make_shared<FunctionCall>(dunder_name, std::move(args));
}

ExprPtr CompoundExpression::instantiate_template_types(const TypePtr& template_type) const
{
    ExprPtr lhs_expr = lhs_expr_->instantiate_template_types(template_type);
    ExprPtr rhs_expr = rhs_expr_->instantiate_template_types(template_type);
    return std::make_unique<CompoundExpression>(std::move(lhs_expr), token_, std::move(rhs_expr));
}

void CompoundExpression::init_subexpressions(const vector<TypePtr>& types)
{
    func_call_->init(types);
}

TypePtr CompoundExpression::type_impl() const
{
    return func_call_->type();
}

VarPtr CompoundExpression::evaluate_impl() const
{
    VarPtr value = func_call_->evaluate();
    lhs_expr_->evaluate()->copy(value);
    return value;
}
