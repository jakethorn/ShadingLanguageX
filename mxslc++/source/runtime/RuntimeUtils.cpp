//
// Created by jaket on 12/06/2026.
//

#include "runtime/RuntimeUtils.h"

#include "runtime/Type.h"
#include "expressions/FunctionCall.h"
#include "expressions/MethodCall.h"
#include "expressions/NamedConstructor.h"
#include "expressions/RuntimeExpression.h"

ExprPtr RuntimeUtils::function_call(string func_name)
{
    return std::make_unique<FunctionCall>(std::move(func_name), std::nullopt);
}

ExprPtr RuntimeUtils::function_call(string func_name, ArgumentList args)
{
    return std::make_unique<FunctionCall>(std::move(func_name), std::move(args));
}

ExprPtr RuntimeUtils::function_call(string func_name, ArgumentList args, Token token)
{
    return std::make_unique<FunctionCall>(std::move(func_name), std::move(args), std::move(token));
}

ExprPtr RuntimeUtils::method_call(VarPtr instance, string func_name)
{
    ExprPtr instance_expr = std::make_unique<RuntimeExpression>(std::move(instance));
    return std::make_unique<MethodCall>(std::move(instance_expr), std::move(func_name), std::nullopt);
}

ExprPtr RuntimeUtils::named_constructor(string ctor_name, ArgumentList args)
{
    return std::make_unique<NamedConstructor>(std::move(ctor_name), std::move(args));
}

VarPtr RuntimeUtils::invoke_function(string func_name, ArgumentList args)
{
    const ExprPtr func_call = function_call(std::move(func_name), std::move(args));
    func_call->init();
    return func_call->evaluate();
}

VarPtr RuntimeUtils::invoke_function(const TypePtr& target_type, string func_name, ArgumentList args)
{
    const ExprPtr func_call = function_call(std::move(func_name), std::move(args));
    func_call->init(target_type);
    return func_call->evaluate();
}

VarPtr RuntimeUtils::invoke_function(const vector<TypePtr>& target_types, string func_name)
{
    const ExprPtr func_call = function_call(std::move(func_name));
    func_call->init(target_types);
    return func_call->evaluate();
}

VarPtr RuntimeUtils::invoke_method(const vector<TypePtr>& target_types, VarPtr instance, string func_name)
{
    const ExprPtr meth_call = method_call(std::move(instance), std::move(func_name));
    meth_call->init(target_types);
    return meth_call->evaluate();
}

VarPtr RuntimeUtils::invoke_constructor(string ctor_name, ArgumentList args)
{
    const ExprPtr ctor = named_constructor(std::move(ctor_name), std::move(args));
    ctor->init();
    return ctor->evaluate();
}

VarPtr RuntimeUtils::invoke_constructor(const TypePtr& target_type, string ctor_name, ArgumentList args)
{
    const ExprPtr ctor = named_constructor(std::move(ctor_name), std::move(args));
    ctor->init(target_type);
    return ctor->evaluate();
}

VarPtr RuntimeUtils::invoke_constructor(const TypePtr& type, ArgumentList args)
{
    return invoke_constructor(type, type->name(), std::move(args));
}
