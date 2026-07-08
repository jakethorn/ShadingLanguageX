//
// Created by jaket on 27/06/2026.
//

#include "errors/AmbiguousFunctionError.h"
#include "runtime/Function.h"

namespace mxslc
{
    AmbiguousFunctionError::AmbiguousFunctionError(const string& message) : CompileError{message}
    {

    }

    AmbiguousFunctionError::AmbiguousFunctionError(const string& func_name, const vector<FuncPtr>& funcs)
        : AmbiguousFunctionError{format(func_name, funcs, {})}
    {

    }

    AmbiguousFunctionError::AmbiguousFunctionError(const string& func_name, const vector<FuncPtr>& funcs, const vector<string>& underlying_errors)
        : AmbiguousFunctionError{format(func_name, funcs, underlying_errors)}
    {

    }

    string AmbiguousFunctionError::format(const string& func_name, const vector<FuncPtr>& funcs, const vector<string>& underlying_errors)
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

        return message;
    }
}
