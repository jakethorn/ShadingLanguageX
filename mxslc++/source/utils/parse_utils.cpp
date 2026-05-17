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
    vector<Token> tokens = sscan(code);
    vector<StmtPtr> stmts = parse(std::move(tokens));
    if (stmts.size() != 1)
        throw CompileError{"Too many statements"s};
    FunctionDefinition* func_def = dynamic_cast<FunctionDefinition*>(stmts[0].get());
    if (func_def == nullptr)
        throw CompileError{"Incorrect statement"s};
    if (func_def->functions().size() != 1)
        throw CompileError{"Templated functions not supported"s};
    return func_def->functions()[0];
}
