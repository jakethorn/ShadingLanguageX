//
// Created by jaket on 16/04/2026.
//

#include "expressions/FunctionCall.h"

#include "expressions/MethodCall.h"
#include "expressions/ThisExpression.h"
#include "expressions/interface.h"
#include "runtime/interface.h"
#include "runtime/Function.h"
#include "runtime/FunctionQuery.h"
#include "runtime/variables/Variable.h"
#include "runtime/Runtime.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"
#include "runtime/utils/FunctionResolver.h"
#include "runtime/utils/monomorphize.h"
#include "serialize/serializer_utils.h"

namespace mxslc::expressions
{
    FunctionCall::FunctionCall(string name)
        : FunctionCall{std::move(name), std::nullopt}
    {

    }

    FunctionCall::FunctionCall(string name, optional<ArgumentList> args)
        : FunctionCall{std::move(name), std::move(args), Token{}}
    {

    }

    FunctionCall::FunctionCall(string name, optional<ArgumentList> args, Token token)
        : FunctionCall{std::move(name), nullptr, std::move(args), std::move(token)}
    {

    }

    FunctionCall::FunctionCall(string name, TypePtr template_type, optional<ArgumentList> args)
        : FunctionCall{std::move(name), std::move(template_type), std::move(args), Token{}}
    {

    }

    FunctionCall::FunctionCall(string name, TypePtr template_type, optional<ArgumentList> args, Token token)
        : FunctionCall{std::move(name), std::move(template_type), std::move(args), AttributeList{}, std::move(token)}
    {

    }

    FunctionCall::FunctionCall(string name, TypePtr template_type, optional<ArgumentList> args, AttributeList attrs)
        : FunctionCall{std::move(name), std::move(template_type), std::move(args), std::move(attrs), Token{}}
    {

    }

    FunctionCall::FunctionCall(string name, TypePtr template_type, optional<ArgumentList> args, AttributeList attrs, Token token)
        : Expression{std::move(token)},
        name_{std::move(name)},
        template_type_{std::move(template_type)},
        args_{std::move(args).value_or(ArgumentList{})},
        is_argumentless_{not args.has_value()}
    {
        set_attributes(std::move(attrs));
    }

    ExprPtr FunctionCall::monomorphize(const TypePtr& template_type) const
    {
        return create_expression<FunctionCall>(
            name_,
            runtime_utils::monomorphize(template_type_, template_type),
            runtime_utils::monomorphize(args_, template_type),
            attrs_,
            token_
        );
    }

    void FunctionCall::init_subexpressions(const vector<TypePtr>& types)
    {

    }

    void FunctionCall::init_impl(const vector<TypePtr>& types)
    {
        if (template_type_)
            template_type_ = scope().resolve_type(template_type_);

        func_ = runtime_utils::resolve_function(types, name_, template_type_, args_, is_argumentless_);

        for (const Argument& arg : args_)
            arg.validate(func_->parameters()[arg]);

        if (func_->has_class_type() and method_call_ == nullptr)
        {
            ExprPtr instance = create_expression<ThisExpression>(token_);
            method_call_ = create_expression<MethodCall>(std::move(instance), std::move(name_), std::move(template_type_), std::move(args_), std::move(attrs_), std::move(token_));
            method_call_->init();
        }
    }

    TypePtr FunctionCall::type_impl() const
    {
        return func_->return_type();
    }

    VarPtr FunctionCall::evaluate_impl() const
    {
        if (func_->has_class_type())
            return method_call_->evaluate();

        if (func_->is_inline())
        {
            runtime().enter_scope();
            evaluate_arguments();
            VarPtr return_value = inline_invoke();
            update_out_arguments();
            runtime().exit_scope();
            return return_value;
        }
        else
        {
            if (func_->is_parameterless())
                return serialize_utils::create_node_graph_value(func_);
            else
                return serializer().write_node(func_, args_, attrs_);
        }
    }

    // inline only
    void FunctionCall::evaluate_arguments() const
    {
        for (const Parameter& param : func_->parameters())
        {
            ModifierList mods = param.modifiers().without(TokenType::Ref, TokenType::Out);
            if (param.is_in())
            {
                const VarPtr arg_value = args_.evaluate(param);
                const VarPtr arg_value_copy = create_variable(std::move(mods), param.type(), arg_value);
                arg_value_copy->disable_node_naming();
                arg_value_copy->add_to_scope(param.name());
            }
            else
            {
                const VarPtr default_value = param.has_default_value() ? param.evaluate() : create_variable(param.type());
                default_value->set_modifiers(std::move(mods));
                default_value->disable_node_naming();
                default_value->add_to_scope(param.name());
            }
        }
    }

    // inline only
    VarPtr FunctionCall::inline_invoke() const
    {
        const VarPtr return_value = func_->invoke();
        if (return_value == nullptr)
            return nullptr;
        return return_value->copy();
    }

    // inline only
    void FunctionCall::update_out_arguments() const
    {
        for (const Parameter& param : func_->parameters())
        {
            if (param.is_out())
            {
                const VarPtr nonlocal = args_.evaluate(param);
                const VarPtr local = scope().get_variable(param.name());
                nonlocal->copy(local);
            }
        }
    }

    string FunctionCall::to_string() const
    {
        const string template_type_string = template_type_ ? "<" + template_type_->to_string() + ">" : "";
        const string args_string = is_argumentless_ ? "" : "(" + join(args_, ", ") + ")";
        return name_ + template_type_string + args_string;
    }
}
