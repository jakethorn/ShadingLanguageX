//
// Created by jaket on 06/11/2025.
//

#ifndef FENNEC_PARSE_H
#define FENNEC_PARSE_H

#include <functional>

#include "utils/common.h"
#include "Token.h"
#include "TokenReader.h"
#include "runtime/Type.h"

class Attribute;
class Parameter;
class Argument;
class Runtime;

vector<StmtPtr> parse(const Runtime& runtime, vector<Token> tokens);

class Parser final : protected TokenReader
{
public:
    explicit Parser(const Runtime& runtime, vector<Token> tokens_);

    vector<StmtPtr> parse();

private:
    Attribute attribute();

    StmtPtr statement();
    vector<Token> modifiers();
    StmtPtr print_statement();
    StmtPtr variable_definition(vector<Token> modifiers);
    StmtPtr function_definition(vector<Token> modifiers);
    StmtPtr function_definition_modern(vector<Token> modifiers);
    Parameter parameter();
    tuple<vector<StmtPtr>, ExprPtr> function_body();

    ExprPtr expression();
    ExprPtr logical();
    ExprPtr equality();
    ExprPtr relational();
    ExprPtr term();
    ExprPtr factor();
    ExprPtr exponent();
    ExprPtr unary();
    ExprPtr property();
    ExprPtr primary();
    ExprPtr function_call();
    Argument argument(size_t i);

    template<typename T>
    vector<T> list(const TokenType opener, const TokenType closer, std::function<T(size_t)> func)
    {
        match(opener);
        if (consume(closer))
            return {};

        vector<T> args;

        do
        {
            args.push_back(func(args.size()));
        }
        while (consume(',') and peek() != closer);

        match(closer);
        return args;
    }


    bool is_templated_function() const;

    const Runtime& runtime_;
    Type current_function_type_;
};

#endif //FENNEC_PARSE_H
