//
// Created by jaket on 06/11/2025.
//

#ifndef FENNEC_PARSE_H
#define FENNEC_PARSE_H

#include <functional>

#include "utils/common.h"
#include "Token.h"
#include "TokenReader.h"
#include "runtime/ModifierList.h"

class Attribute;
class Parameter;
class Argument;
class Runtime;
class FieldInfo;

vector<StmtPtr> parse(const Runtime& runtime, vector<Token> tokens);

class Parser final : protected TokenReader
{
public:
    Parser(const Runtime& runtime, vector<Token> tokens_);

    vector<StmtPtr> parse();

private:
    Attribute attribute();

    StmtPtr statement();
    StmtPtr print_statement();
    StmtPtr variable_definition(ModifierList mods, TypeInfoPtr type);
    StmtPtr multi_variable_definition(ModifierList mods, TypeInfoPtr type);
    StmtPtr function_definition(ModifierList mods, TypeInfoPtr type);
    StmtPtr function_definition_modern(ModifierList mods);
    StmtPtr using_declaration();

    ModifierList modifiers();
    TypeInfoPtr type_info();
    FieldInfo field_info();
    Parameter parameter(size_t index);
    vector<TypeInfoPtr> template_list();
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
    ExprPtr named_constructor();
    ExprPtr unnamed_constructor();

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

    [[nodiscard]] bool is_templated_function() const;

    const Runtime& runtime_;
};

#endif //FENNEC_PARSE_H
