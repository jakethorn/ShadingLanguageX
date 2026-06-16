//
// Created by jaket on 12/06/2026.
//

#ifndef MXSLC_RUNTIMEUTILS_H
#define MXSLC_RUNTIMEUTILS_H

#include "Token.h"
#include "utils/common.h"

class ArgumentList;

class RuntimeUtils
{
public:
    static ExprPtr function_call(string func_name, ArgumentList args);
    static ExprPtr function_call(string func_name, ArgumentList args, Token token);
    static ExprPtr named_constructor(string ctor_name, ArgumentList args);

    static VarPtr invoke_constructor(string ctor_name, ArgumentList args);
    static VarPtr invoke_constructor(string ctor_name, ArgumentList args, const TypePtr& target_type);
    static VarPtr invoke_constructor(const TypePtr& type, ArgumentList args);

    static VarPtr invoke_function(string func_name, ArgumentList args);
    static VarPtr invoke_function(string func_name, ArgumentList args, const TypePtr& target_type);
};

#endif //MXSLC_RUNTIMEUTILS_H
