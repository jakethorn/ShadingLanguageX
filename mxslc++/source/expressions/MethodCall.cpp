//
// Created by jaket on 06/05/2026.
//

#include <cassert>

#include "expressions/MethodCall.h"

#include "runtime/Function.h"
#include "runtime/FunctionQuery.h"
#include "runtime/Runtime.h"
#include "runtime/Scope.h"
#include "runtime/utils/FunctionResolver.h"
#include "runtime/variables/Variable.h"
#include "serialize/serializer_utils.h"
#include "serialize/values/interface.h"

namespace mxslc::expressions
{
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
    }

    void MethodCall::init_impl(const vector<TypePtr>& types)
    {
        if (template_type_)
            template_type_ = scope().resolve_type(template_type_);

        func_ = runtime_utils::resolve_method(instance_->type(), types, name_, template_type_, args_, is_argumentless_);

        for (const Argument& arg : args_)
        {
            assert(arg.is_initialized());
            arg.validate(func_->parameters()[arg]);
        }
    }

    VarPtr MethodCall::evaluate_impl() const
    {
        if (func_->is_inline())
        {
            runtime().enter_scope(name_);
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
                return serialize_utils::create_node_graph_value(func_);
            else
                return serializer().write_node(instance_, func_, args_, attrs_);
        }
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

    string MethodCall::to_string() const
    {
        return instance_expr_->to_string() + "." + FunctionCall::to_string();
    }
}
