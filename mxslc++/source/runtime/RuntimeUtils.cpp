//
// Created by jaket on 12/06/2026.
//

#include "RuntimeUtils.h"

#include "Type.h"
#include "expressions/FunctionCall.h"
#include "expressions/NamedConstructor.h"

ExprPtr RuntimeUtils::function_call(string func_name, ArgumentList args)
{
    return std::make_unique<FunctionCall>(std::move(func_name), std::move(args));
}

ExprPtr RuntimeUtils::named_constructor(string ctor_name, ArgumentList args)
{
    return std::make_unique<NamedConstructor>(std::move(ctor_name), std::move(args));
}

VarPtr RuntimeUtils::invoke_constructor(string ctor_name, ArgumentList args)
{
    const ExprPtr ctor = named_constructor(std::move(ctor_name), std::move(args));
    ctor->init();
    return ctor->evaluate();
}

VarPtr RuntimeUtils::invoke_constructor(string ctor_name, ArgumentList args, const TypePtr& target_type)
{
    const ExprPtr ctor = named_constructor(std::move(ctor_name), std::move(args));
    ctor->init(target_type);
    return ctor->evaluate();
}

VarPtr RuntimeUtils::invoke_constructor(const TypePtr& type, ArgumentList args)
{
    return invoke_constructor(type->name(), std::move(args), type);
}

VarPtr RuntimeUtils::invoke_function(string func_name, ArgumentList args)
{
    const ExprPtr func_call = function_call(std::move(func_name), std::move(args));
    func_call->init();
    return func_call->evaluate();
}

VarPtr RuntimeUtils::invoke_function(string func_name, ArgumentList args, const TypePtr& target_type)
{
    const ExprPtr func_call = function_call(std::move(func_name), std::move(args));
    func_call->init(target_type);
    return func_call->evaluate();
}
