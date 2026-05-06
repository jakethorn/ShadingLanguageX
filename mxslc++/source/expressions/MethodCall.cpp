//
// Created by jaket on 06/05/2026.
//

#include "MethodCall.h"

#include "runtime/function_utils.h"
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

void MethodCall::init_impl(const vector<TypePtr>& types)
{
    const VarPtr instance = instance_expr_->evaluate();

    func_ = get_matching_function(instance->type()->methods(), types, name_, template_type_, args_);

    for (const Argument& arg : args_)
        arg.validate(func_->parameters()[arg]);
}

VarPtr MethodCall::evaluate_impl() const
{
    set_this();
    return FunctionCall::evaluate_impl();
}

void MethodCall::set_this() const
{
    const VarPtr instance = instance_expr_->evaluate();
    for (const auto& [_, nonlocal_input] : func_->nonlocal_inputs())
    {
        if (nonlocal_input->name() == "__this__"s)
            nonlocal_input->copy_value(instance);
    }
    for (const auto& [_, nonlocal_output] : func_->nonlocal_outputs())
    {
        if (nonlocal_output->name() == "__this__"s)
            nonlocal_output->copy_value(instance);
    }
}
