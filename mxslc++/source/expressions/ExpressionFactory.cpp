//
// Created by jaket on 08/12/2025.
//

#include "expressions/ExpressionFactory.h"
#include "expressions/FunctionCall.h"
#include "runtime/ArgumentList.h"
#include "runtime/RuntimeUtils.h"

ExprPtr ExpressionFactory::binary(ExprPtr left, Token op, ExprPtr right)
{
    static const unordered_map<TokenType, string> op_names {
        {'+', "__add__"},
        {'-', "__sub__"},
        {'*', "__mul__"},
        {'/', "__div__"},
        {'%', "__mod__"},
        {'^', "__pow__"},
        {"=="s, "__eq__"},
        {"!="s, "__ne__"},
        {'>', "__gt__"},
        {'<', "__lt__"},
        {">="s, "__ge__"},
        {"<="s, "__le__"},
        {'&', "__and__"},
        {'|', "__or__"},
    };

    string dunder_name = op_names.at(op.type());
    ArgumentList args{std::move(left), std::move(right)};
    return RuntimeUtils::function_call(std::move(dunder_name), std::move(args), std::move(op));
}

ExprPtr ExpressionFactory::ternary_relational(ExprPtr left, Token op1, ExprPtr middle, Token op2, ExprPtr right)
{
    static const unordered_map<TokenType, string> op_names {
        {"=="s, "eq"},
        {"!="s, "ne"},
        {'>', "gt"},
        {'<', "lt"},
        {">="s, "ge"},
        {"<="s, "le"},
    };

    string dunder_name = "__" + op_names.at(op1.type()) + "_" + op_names.at(op2.type()) + "__";
    ArgumentList args{std::move(left), std::move(middle), std::move(right)};
    return RuntimeUtils::function_call(std::move(dunder_name), std::move(args), std::move(op1));
}

ExprPtr ExpressionFactory::unary(Token op, ExprPtr right)
{
    static const unordered_map<TokenType, string> op_names {
        {'+', "__pos__"},
        {'-', "__neg__"},
        {'!', "__not__"},
    };

    string dunder_name = op_names.at(op.type());
    return RuntimeUtils::function_call(std::move(dunder_name), ArgumentList{std::move(right)}, std::move(op));
}

ExprPtr ExpressionFactory::if_expression(ExprPtr cond_expr, ExprPtr then_expr, ExprPtr else_expr, Token token)
{
    ArgumentList args{std::move(cond_expr), std::move(then_expr), std::move(else_expr)};
    return RuntimeUtils::function_call("__if__", std::move(args), std::move(token));
}

ExprPtr ExpressionFactory::absolute_value(ExprPtr expr, Token token)
{

    return RuntimeUtils::function_call("__abs__", ArgumentList{std::move(expr)}, std::move(token));
}
