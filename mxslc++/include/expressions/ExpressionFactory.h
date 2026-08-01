//
// Created by jaket on 08/12/2025.
//

#ifndef FENNEC_EXPRESSIONFACTORY_H
#define FENNEC_EXPRESSIONFACTORY_H

#include "common.h"
#include "Token.h"

namespace mxslc::expressions
{
    class ExpressionFactory
    {
    public:
        static ExprPtr binary(ExprPtr left, Token op, ExprPtr right);
        static ExprPtr ternary_relational(ExprPtr left, Token op1, ExprPtr middle, Token op2, ExprPtr right);
        static ExprPtr unary(Token op, ExprPtr right);
        static ExprPtr absolute(ExprPtr expr, Token token);

    private:
        ExpressionFactory() = default;
        ~ExpressionFactory() = default;
    };
}

#endif //FENNEC_EXPRESSIONFACTORY_H
