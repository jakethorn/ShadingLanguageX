//
// Created by jaket on 12/06/2026.
//

#include "RuntimeUtils.h"

#include "expressions/FunctionCall.h"

VarPtr RuntimeUtils::invoke_function(string func_name, ArgumentList args)
{
    const ExprPtr func_call = std::make_unique<FunctionCall>(std::move(func_name), std::move(args));
    func_call->init();
    return func_call->evaluate();
}
