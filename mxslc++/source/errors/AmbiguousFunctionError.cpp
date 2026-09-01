//
// Created by jaket on 27/06/2026.
//

#include "errors/AmbiguousFunctionError.h"
#include "runtime/Function.h"
#include "runtime/FunctionQuery.h"
#include "runtime/Type.h"

namespace mxslc
{
    AmbiguousFunctionError::AmbiguousFunctionError(const string& message) : CompileError{message}
    {

    }

    AmbiguousFunctionError::AmbiguousFunctionError(const FunctionQuery& query, const vector<FuncPtr>& funcs)
        : AmbiguousFunctionError{format(query, funcs)}
    {

    }

    string AmbiguousFunctionError::format(const FunctionQuery& query, const vector<FuncPtr>& funcs)
    {
        const string func_type = query.has_class_type() ? "method" : "function";
        const string func_name = query.has_class_type() ? (*query.class_type)->to_string() + "." + *query.name : *query.name;

        string message;
        if (funcs.empty())
        {
            message = "No " + func_type + " could be found with the name '" + func_name + "'";
        }
        else if (funcs.size() == 1)
        {
            message = "No " + func_type + " could be found with the signature '" + query.to_string() + "'";
            message += "\nPossible match:\n";
            for (const FuncPtr& func : funcs)
                message += func->header() + "\n";
        }
        else
        {
            message = "The " + func_type + " signature '" + query.to_string() + "' is ambiguous";
            message += "\nPossible matches:\n";
            for (const FuncPtr& func : funcs)
                message += func->header() + "\n";
        }

        return message;
    }
}
