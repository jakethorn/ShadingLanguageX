//
// Created by jaket on 10/04/2026.
//

#include "error_utils.h"

#include "AmbiguousFunctionError.h"
#include "statements/Statement.h"
#include "runtime/Function.h"

AmbiguousFunctionError ambiguous_function_error(const string& func_name, const vector<FuncPtr>& funcs)
{
    return ambiguous_function_error(func_name, funcs, {});
}

AmbiguousFunctionError ambiguous_function_error(const string& func_name, const vector<FuncPtr>& funcs, const vector<string>& underlying_errors)
{
    string message;
    if (funcs.empty())
    {
        message = "Definition could not be found for function '" + func_name + "'";
    }
    else
    {
        message = "Matching definition could not be found for function '" + func_name + "'\n";
        message += "Possible matches:\n";
        for (const FuncPtr& func : funcs)
            message += func->str() + "\n";
    }

    if (not underlying_errors.empty())
        message += "\nUnderlying errors:\n";

    for (const string& underlying_error : underlying_errors)
        message += "\n" + underlying_error;

    return AmbiguousFunctionError{message};
}
