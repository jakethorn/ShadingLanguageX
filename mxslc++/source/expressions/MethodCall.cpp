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
    Runtime::get().enter_scope();
    set_this();
    VarPtr return_value = FunctionCall::evaluate_impl();
    Runtime::get().exit_scope();
    return return_value;
}

vector<FuncPtr> MethodCall::get_matching_functions(const vector<TypePtr>& return_types) const
{
    const string method_name = instance_->type()->name() + "__" + name_;
    return ::get_matching_functions(lock(instance_->type()->methods()), return_types, method_name, template_type_, args_);
}

FuncPtr MethodCall::get_matching_function(const vector<TypePtr>& return_types) const
{
    const string method_name = instance_->type()->name() + "__" + name_;
    return ::get_matching_function(lock(instance_->type()->methods()), return_types, method_name, template_type_, args_);
}

namespace
{
    vector<size_t> ancestral_path(VarPtr child)
    {
        vector<size_t> indices;
        while (child->has_parent())
        {
            indices.insert(indices.cbegin(), child->sibling_index());
            child = child->parent();
        }
        return indices;
    }
}

void MethodCall::set_this() const
{
    scope().set_this(Variable::create(instance_));

    for (const VarPtr& nonlocal_input : func_->nonlocal_inputs())
    {
        if (nonlocal_input->oldest()->is_this())
        {
            VarPtr var = instance_;
            for (const size_t index : ancestral_path(nonlocal_input))
                var = var->child(index);
            nonlocal_input->uninitialize();
            nonlocal_input->copy_value(var);
        }
    }
    for (const VarPtr& nonlocal_output : func_->nonlocal_outputs())
    {
        if (nonlocal_output->oldest()->is_this())
        {
            VarPtr var = instance_;
            for (const size_t index : ancestral_path(nonlocal_output))
                var = var->child(index);
            nonlocal_output->uninitialize();
            nonlocal_output->copy_value(var);
        }
    }
}
