//
// Created by jaket on 08/12/2025.
//

#ifndef FENNEC_EXPRESSIONFACTORY_H
#define FENNEC_EXPRESSIONFACTORY_H

#include "utils/common.h"
#include "Token.h"

class Runtime;

class ExpressionFactory
{
public:
    static void init(const Runtime& runtime);
    static ExprPtr binary(ExprPtr left, Token op, ExprPtr right);
    static ExprPtr ternary_relational(ExprPtr left, Token op1, ExprPtr middle, Token op2, ExprPtr right);
    static ExprPtr unary(Token op, ExprPtr right);

private:
    ExpressionFactory() = default;
    ~ExpressionFactory() = default;

    static const Runtime* runtime_;
};

#endif //FENNEC_EXPRESSIONFACTORY_H
