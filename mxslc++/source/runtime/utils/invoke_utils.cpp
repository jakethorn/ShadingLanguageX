//
// Created by jaket on 12/06/2026.
//

#include "runtime/utils/invoke_utils.h"

#include "runtime/ArgumentList.h"
#include "runtime/Type.h"
#include "expressions/FunctionCall.h"
#include "expressions/interface.h"
#include "expressions/MethodCall.h"
#include "expressions/NamedConstructor.h"
#include "expressions/RuntimeExpression.h"

namespace mxslc::runtime::runtime_utils
{
    VarPtr invoke_function(string func_name, ArgumentList args)
    {
        const FunctionCallPtr func_call = create_expression<FunctionCall>(std::move(func_name), std::move(args));

        func_call->init();
        return func_call->evaluate();
    }

    VarPtr invoke_function(const TypePtr& target_type, string func_name, ArgumentList args)
    {
        const FunctionCallPtr func_call = create_expression<FunctionCall>(std::move(func_name), std::move(args));

        func_call->init(target_type);
        return func_call->evaluate();
    }

    VarPtr invoke_function(const vector<TypePtr>& target_types, string func_name)
    {
        const FunctionCallPtr func_call = create_expression<FunctionCall>(std::move(func_name));

        func_call->init(target_types);
        return func_call->evaluate();
    }

    VarPtr invoke_method(const vector<TypePtr>& target_types, VarPtr instance, string func_name)
    {
        ExprPtr instance_expr = create_expression<RuntimeExpression>(std::move(instance));
        const ExprPtr method_call = create_expression<MethodCall>(std::move(instance_expr), std::move(func_name), std::nullopt);

        method_call->init(target_types);
        return method_call->evaluate();
    }

    VarPtr invoke_constructor(string ctor_name, ArgumentList args)
    {
        const NamedConstructorPtr ctor = create_expression<NamedConstructor>(std::move(ctor_name), std::move(args));

        ctor->init();
        return ctor->evaluate();
    }

    VarPtr invoke_constructor(const TypePtr& target_type, string ctor_name, ArgumentList args)
    {
        const NamedConstructorPtr ctor = create_expression<NamedConstructor>(std::move(ctor_name), std::move(args));

        ctor->init(target_type);
        return ctor->evaluate();
    }

    VarPtr invoke_constructor(const TypePtr& type, ArgumentList args)
    {
        return invoke_constructor(type, type->name(), std::move(args));
    }
}
