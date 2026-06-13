//
// Created by jaket on 17/05/2026.
//

#include "parse_utils.h"
#include "scan.h"
#include "parse.h"
#include "statements/FunctionDefinition.h"
#include "CompileError.h"

FuncPtr parse_function(const string& code)
{
    vector<Token> tokens = scan_string(code);
    const vector<StmtPtr> stmts = parse(std::move(tokens));
    if (stmts.size() != 1)
        throw CompileError{"Too many statements"s};
    const FunctionDefinition* func_def = dynamic_cast<FunctionDefinition*>(stmts[0].get());
    if (func_def == nullptr)
        throw CompileError{"Incorrect statement"s};
    if (func_def->functions().size() != 1)
        throw CompileError{"Templated functions not supported"s};
    return func_def->functions()[0];
}

primitive_t parse_literal(const string& code)
{
    const vector<Token> tokens = scan_string(code);
    if (tokens.size() == 1)
        return tokens[0].literal();
    if (tokens.size() == 2)
    {
        const primitive_t value = tokens[1].literal();
        if (tokens[0] == '-')
        {
            if (std::holds_alternative<int>(value))
                return -std::get<int>(value);
            if (std::holds_alternative<float>(value))
                return -std::get<float>(value);
        }
    }

    throw CompileError{"Invalid literal: " + code};
}
