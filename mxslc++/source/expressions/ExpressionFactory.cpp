//
// Created by jaket on 08/12/2025.
//

#include "ExpressionFactory.h"
#include "utils/common.h"
#include "FunctionCall.h"

const Runtime* ExpressionFactory::runtime_ = nullptr;

void ExpressionFactory::init(const Runtime& runtime)
{
    runtime_ = &runtime;
}

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
        {"&"s, "__and__"},
        {"|"s, "__or__"},
    };

    op.set_lexeme(op_names.at(op.type()));

    ArgumentList args{std::move(left), std::move(right)};
    return std::make_unique<FunctionCall>(*runtime_, std::move(op), std::move(args));
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

    const string s = "__" + op_names.at(op1.type()) + "_" + op_names.at(op2.type()) + "__";
    op1.set_lexeme(s);

    ArgumentList args{std::move(left), std::move(middle), std::move(right)};
    return std::make_unique<FunctionCall>(*runtime_, std::move(op1), std::move(args));
}

ExprPtr ExpressionFactory::unary(Token op, ExprPtr right)
{
    static const unordered_map<TokenType, string> op_names {
        {'+', "__pos__"},
        {'-', "__neg__"},
        {'!', "__not__"},
    };

    op.set_lexeme(op_names.at(op.type()));

    ArgumentList args{std::move(right)};
    return std::make_unique<FunctionCall>(*runtime_, std::move(op), std::move(args));
}

ExprPtr ExpressionFactory::named_constructor(Token name, vector<Argument> arguments)
{
    name.set_lexeme("__" + name.lexeme() + "__");
    return std::make_unique<FunctionCall>(*runtime_, std::move(name), std::move(arguments));
}
