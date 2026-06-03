//
// Created by jaket on 10/04/2026.
//

#include "error_utils.h"

#include <cassert>

#include "statements/Statement.h"
#include "runtime/ArgumentList.h"
#include "runtime/Function.h"
#include "runtime/Function.h"
#include "runtime/Type.h"

CompileError ambiguous_function_error(const string& func_name, const vector<FuncPtr>& funcs)
{
    if (funcs.empty())
    {
        return CompileError{"Definition could not be found for function '" + func_name + "'"};
    }
    else
    {
        string result = "Matching definition could not be found for function '" + func_name + "'\n";
        result += "Possible matches:\n";
        for (const FuncPtr& func : funcs)
            result += func->str() + "\n";
        return CompileError{result};
    }
}
