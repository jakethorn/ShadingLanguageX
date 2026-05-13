//
// Created by jaket on 06/05/2026.
//

#include "MethodCall.h"

#include "runtime/Function.h"
#include "runtime/function_utils.h"
#include "runtime/Runtime.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"
#include "runtime/Variable.h"

MethodCall::MethodCall(ExprPtr instance_expr, string method_name, TypePtr template_type, ArgumentList args)
    : MethodCall{std::move(instance_expr), std::move(method_name), std::move(template_type), std::move(args), Token{}}
{

}

MethodCall::MethodCall(ExprPtr instance_expr, string method_name, TypePtr template_type, ArgumentList args, Token token)
    : MethodCall{std::move(instance_expr), std::move(method_name), std::move(template_type), std::move(args), AttributeList{}, std::move(token)}
{

}

MethodCall::MethodCall(ExprPtr instance_expr, string method_name, TypePtr template_type, ArgumentList args, AttributeList attrs)
    : MethodCall{std::move(instance_expr), std::move(method_name), std::move(template_type), std::move(args), std::move(attrs), Token{}}
{

}

MethodCall::MethodCall(ExprPtr instance_expr, string method_name, TypePtr template_type, ArgumentList args, AttributeList attrs, Token token)
    : FunctionCall{std::move(method_name), std::move(template_type), std::move(args), std::move(attrs), std::move(token)}, instance_expr_{std::move(instance_expr)}
{

}

void MethodCall::init_subexpressions(const vector<TypePtr>& types)
{
    instance_expr_->init();
    instance_ = instance_expr_->evaluate();
    FunctionCall::init_subexpressions(types);
}

VarPtr MethodCall::evaluate_impl() const
{
    if (func_->is_inline())
    {
        Runtime::get().enter_scope();
        evaluate_arguments();
        VarPtr return_value = func_->invoke();
        update_out_arguments();
        Runtime::get().exit_scope();
        return return_value;
    }
    else
    {
        return serializer().write_node(instance_, func_, args_, attrs_);
    }
}

vector<FuncPtr> MethodCall::get_matching_functions(const vector<TypePtr>& return_types) const
{
    return ::get_matching_functions(instance_->type()->methods(), return_types, name_, template_type_, args_);
}

FuncPtr MethodCall::get_matching_function(const vector<TypePtr>& return_types) const
{
    return ::get_matching_function(instance_->type()->methods(), return_types, name_, template_type_, args_);
}
