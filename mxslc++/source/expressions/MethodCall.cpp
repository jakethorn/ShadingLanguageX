//
// Created by jaket on 06/05/2026.
//

#include "MethodCall.h"

#include "runtime/Function.h"
#include "runtime/FunctionQuery.h"
#include "runtime/Runtime.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"
#include "runtime/Variable.h"
#include "values/ValueFactory.h"

MethodCall::MethodCall(ExprPtr instance_expr, string method_name, optional<ArgumentList> args)
    : MethodCall{std::move(instance_expr), std::move(method_name), nullptr, std::move(args)}
{

}

MethodCall::MethodCall(ExprPtr instance_expr, string method_name, TypePtr template_type, optional<ArgumentList> args)
    : MethodCall{std::move(instance_expr), std::move(method_name), std::move(template_type), std::move(args), Token{}}
{

}

MethodCall::MethodCall(ExprPtr instance_expr, string method_name, TypePtr template_type, optional<ArgumentList> args, Token token)
    : MethodCall{std::move(instance_expr), std::move(method_name), std::move(template_type), std::move(args), AttributeList{}, std::move(token)}
{

}

MethodCall::MethodCall(ExprPtr instance_expr, string method_name, TypePtr template_type, optional<ArgumentList> args, AttributeList attrs)
    : MethodCall{std::move(instance_expr), std::move(method_name), std::move(template_type), std::move(args), std::move(attrs), Token{}}
{

}

MethodCall::MethodCall(ExprPtr instance_expr, string method_name, TypePtr template_type, optional<ArgumentList> args, AttributeList attrs, Token token)
    : FunctionCall{std::move(method_name), std::move(template_type), std::move(args), std::move(attrs), std::move(token)}, instance_expr_{std::move(instance_expr)}
{

}

void MethodCall::init_subexpressions(const vector<TypePtr>& types)
{
    instance_expr_->init();
    instance_ = instance_expr_->evaluate();
    FunctionCall::init_subexpressions(types);
}

void MethodCall::init_impl(const vector<TypePtr> &types)
{
    func_ = get_matching_function(scope(), types);
    for (const Argument& arg : args_)
        arg.validate(func_->parameters()[arg]);
}

VarPtr MethodCall::evaluate_impl() const
{
    if (func_->is_inline())
    {
        runtime().enter_scope();
        evaluate_arguments();
        const VarPtr local_instance = copy_instance_to_scope();
        VarPtr return_value = inline_invoke();
        update_out_arguments();
        update_instance(local_instance);
        runtime().exit_scope();
        return return_value;
    }
    else
    {
        if (func_->is_parameterless())
            return ValueFactory::create_node_graph_value(func_);
        else
            return serializer().write_node(instance_, func_, args_, attrs_);
    }
}

vector<FuncPtr> MethodCall::get_matching_functions(const Scope& scope, const vector<TypePtr>& return_types) const
{
    return scope.get_functions({instance_->type(), return_types, name_, template_type_, args_, is_argumentless_});
}

FuncPtr MethodCall::get_matching_function(const Scope& scope, const vector<TypePtr>& return_types) const
{
    return scope.get_function({instance_->type(), return_types, name_, template_type_, args_, is_argumentless_});
}

VarPtr MethodCall::copy_instance_to_scope() const
{
    VarPtr instance_copy = instance_->copy();
    instance_copy->set_modifiers(TokenType::Mutable);
    instance_copy->add_to_scope("this");
    return instance_copy;
}

void MethodCall::update_instance(const VarPtr& local_copy) const
{
    if (not instance_->equals(local_copy))
        instance_->copy(local_copy);
}
