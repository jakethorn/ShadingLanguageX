//
// Created by jaket on 08/12/2025.
//

#include "expressions/ExpressionFactory.h"
#include "expressions/FunctionCall.h"
#include "runtime/ArgumentList.h"
#include "expressions/interface.h"

namespace mxslc::expressions
{
    ExprPtr ExpressionFactory::binary(ExprPtr left, Token op, ExprPtr right)
    {
        static const unordered_map<TokenType, string> OP_NAMES {
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

        string dunder_name = OP_NAMES.at(op.type());
        ArgumentList args{std::move(left), std::move(right)};
        return create_expression<FunctionCall>(std::move(dunder_name), std::move(args), std::move(op));
    }

    ExprPtr ExpressionFactory::ternary_relational(ExprPtr left, Token op1, ExprPtr middle, Token op2, ExprPtr right)
    {
        static const unordered_map<TokenType, string> OP_NAMES {
            {"=="s, "eq"},
            {"!="s, "ne"},
            {'>', "gt"},
            {'<', "lt"},
            {">="s, "ge"},
            {"<="s, "le"},
        };

        string dunder_name = "__" + OP_NAMES.at(op1.type()) + "_" + OP_NAMES.at(op2.type()) + "__";
        ArgumentList args{std::move(left), std::move(middle), std::move(right)};
        return create_expression<FunctionCall>(std::move(dunder_name), std::move(args), std::move(op1));
    }

    ExprPtr ExpressionFactory::unary(Token op, ExprPtr right)
    {
        static const unordered_map<TokenType, string> OP_NAMES {
            {'+', "__pos__"},
            {'-', "__neg__"},
            {'!', "__not__"},
        };

        string dunder_name = OP_NAMES.at(op.type());
        return create_expression<FunctionCall>(std::move(dunder_name), ArgumentList{std::move(right)}, std::move(op));
    }

    ExprPtr ExpressionFactory::if_expression(ExprPtr cond_expr, ExprPtr then_expr, ExprPtr else_expr, Token token)
    {
        ArgumentList args{std::move(cond_expr), std::move(then_expr), std::move(else_expr)};
        return create_expression<FunctionCall>("__if__", std::move(args), std::move(token));
    }

    ExprPtr ExpressionFactory::absolute(ExprPtr expr, Token token)
    {
        return create_expression<FunctionCall>("__abs__", ArgumentList{std::move(expr)}, std::move(token));
    }
}
