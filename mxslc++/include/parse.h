//
// Created by jaket on 06/11/2025.
//

#ifndef FENNEC_PARSE_H
#define FENNEC_PARSE_H

#include "common.h"
#include "TokenReader.h"
#include "runtime/Attribute.h"
#include "runtime/AttributeList.h"
#include "runtime/ModifierList.h"
#include "runtime/Field.h"
#include "runtime/Parameter.h"
#include "runtime/Argument.h"

namespace mxslc
{
    vector<StmtPtr> parse(vector<Token> tokens);

    class Parser final : protected TokenReader
    {
    public:
        explicit Parser(vector<Token> tokens);

        vector<StmtPtr> parse();

        StmtPtr statement();
        StmtPtr bare_statement();
        StmtPtr print_statement();
        StmtPtr variable_definition(ModifierList mods, TypePtr type);
        StmtPtr multi_variable_definition(ModifierList mods, TypePtr type_);
        StmtPtr variable_assignment(ExprPtr lhs);
        StmtPtr function_definition(ModifierList mods, TypePtr type);
        StmtPtr class_definition();
        StmtPtr constructor_definition();
        StmtPtr using_declaration();
        StmtPtr for_loop();
        StmtPtr expression_statement(ExprPtr expr);
        StmtPtr block_statement();
        StmtPtr if_statement();
        StmtPtr document_attribute();

        Attribute attribute();
        AttributeList attributes();
        ModifierList modifiers();
        TypePtr type();
        Field field();
        Parameter parameter(size_t index);
        vector<TypePtr> template_list();
        StmtPtr function_body();
        StmtPtr return_statement();

        ExprPtr expression(const ModifierList& mods = {});
        ExprPtr logical();
        ExprPtr equality();
        ExprPtr relational();
        ExprPtr range();
        ExprPtr term();
        ExprPtr factor();
        ExprPtr exponent();
        ExprPtr unary();
        ExprPtr compound();
        ExprPtr increment();
        ExprPtr property();
        ExprPtr primary();
        ExprPtr if_expression(ExprPtr else_expr = nullptr);
        ExprPtr function_call();
        ExprPtr method_call(ExprPtr instance);
        ExprPtr named_constructor();
        ExprPtr unnamed_constructor();
        ExprPtr variable_definition_argument(ModifierList mods);
        ExprPtr typeof_operator();

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

        template<typename T, typename TList>
        optional<TList> optional_list(const TokenType opener, const TokenType closer, std::function<T(size_t)> func)
        {
            if (peek() != opener)
                return std::nullopt;
            return TList{list(opener, closer, func)};
        }

        bool is_typed_definition() const;
        bool is_function_call() const;
        bool is_constructor_definition() const;

        bool in_abs_{false};
    };
}

#endif //FENNEC_PARSE_H
