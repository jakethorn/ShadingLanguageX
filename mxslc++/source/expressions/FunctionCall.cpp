//
// Created by jaket on 16/04/2026.
//

#include <cassert>

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
#include "utils/container_utils.h"

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

    void FunctionCall::init_impl(const vector<TypePtr>& types)
    {
        if (method_call_)
        {
            method_call_->init(types);
            return;
        }

        if (template_type_)
            template_type_ = scope().resolve_type(template_type_);

        func_ = runtime_utils::resolve_function(types, name_, template_type_, args_, is_argumentless_);

        validate_arguments();

        if (func_->has_class_type() and method_call_ == nullptr)
        {
            ExprPtr instance = create_expression<ThisExpression>(token_);
            method_call_ = create_expression<MethodCall>(std::move(instance), std::move(name_), std::move(template_type_), std::move(args_), std::move(attrs_), std::move(token_));
            method_call_->init(types);
        }
    }

    TypePtr FunctionCall::type_impl() const
    {
        if (method_call_)
            return method_call_->type();

        return func_->return_type();
    }

    VarPtr FunctionCall::evaluate_impl() const
    {
        if (method_call_)
            return method_call_->evaluate();

        if (func_->is_inline())
        {
            runtime().enter_scope(func_);
            evaluate_arguments();
            VarPtr return_value = inline_invoke();
            update_out_arguments();
            runtime().exit_scope();
            return return_value;
        }
        else
        {
            const ConstFunctionCallPtr self = shared_from_child<FunctionCall>();
            if (func_->is_nodegraph())
                return serializer().write_node_graph_value(self);
            else
                return serializer().write_node(self);
        }
    }

    void FunctionCall::validate_arguments() const
    {
        for (const Argument& arg : args_)
        {
            const Parameter& param = func_->parameters()[arg];

            assert(arg.is_initialized());
            arg.validate(param);

            if (func_->is_nodegraph() and not param.is_out())
                throw CompileError{"Arguments cannot be passed to nodegraph functions"};
        }
    }

    // inline only
    void FunctionCall::evaluate_arguments() const
    {
        serializer().begin_comptime(func_->is_comptime());

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

        serializer().end_comptime();
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
        serializer().begin_comptime(func_->is_comptime());

        for (const Parameter& param : func_->parameters())
        {
            if (param.is_out())
            {
                const VarPtr nonlocal = args_.evaluate(param);
                const VarPtr local = scope().get_variable(param.name());
                nonlocal->copy(local);
            }
        }

        serializer().end_comptime();
    }

    int FunctionCall::precedence() const
    {
        if (args_.size() == 2)
        {
            if (name_ == "__or__" || name_ == "__and__")
                return 1;
            if (name_ == "__eq__" || name_ == "__ne__")
                return 2;
            if (name_ == "__lt__" || name_ == "__le__" || name_ == "__gt__" || name_ == "__ge__")
                return 3;
            if (name_ == "__add__" || name_ == "__sub__")
                return 5;
            if (name_ == "__mul__" || name_ == "__div__" || name_ == "__mod__")
                return 6;
            if (name_ == "__pow__")
                return 7;
        }

        if (args_.size() == 1)
        {
            if (name_ == "__not__" || name_ == "__neg__" || name_ == "__pos__")
                return 8;
        }

        return 9;
    }

    string FunctionCall::to_string() const
    {
        static const unordered_map<string, string> BINARY_OPS {
            {"__add__", "+"},
            {"__sub__", "-"},
            {"__mul__", "*"},
            {"__div__", "/"},
            {"__mod__", "%"},
            {"__pow__", "^"},
            {"__eq__", "=="},
            {"__ne__", "!="},
            {"__gt__", ">"},
            {"__lt__", "<"},
            {"__ge__", ">="},
            {"__le__", "<="},
            {"__and__", "&"},
            {"__or__", "|"},
        };

        if (args_.size() == 2 and container_utils::contains(BINARY_OPS, name_))
        {
            const int prec = precedence();
            const auto& left = args_[0];
            const auto& right = args_[1];

            string left_str = left.to_string();
            if (left.precedence() < prec)
                left_str = "(" + left_str + ")";

            string right_str = right.to_string();
            if (right.precedence() < prec)
            {
                right_str = "(" + right_str + ")";
            }
            else if (right.precedence() == prec)
            {
                auto right_func = cast_expression<FunctionCall>(right.expr());
                const bool is_associative = (name_ == "__add__" && right_func && right_func->name() == "__add__") ||
                                            (name_ == "__mul__" && right_func && right_func->name() == "__mul__");
                if (!is_associative)
                    right_str = "(" + right_str + ")";
            }

            return left_str + " " + BINARY_OPS.at(name_) + " " + right_str;
        }

        if (args_.size() == 1 and (name_ == "__not__" or name_ == "__neg__" or name_ == "__pos__"))
        {
            static const unordered_map<string, string> UNARY_OPS {
                {"__not__", "!"},
                {"__neg__", "-"},
                {"__pos__", "+"},
            };
            const string op = UNARY_OPS.at(name_);
            string child_str = args_[0].to_string();
            if (args_[0].precedence() < 8)
                child_str = "(" + child_str + ")";
            return op + child_str;
        }

        const string template_type_string = template_type_ ? "<" + template_type_->to_string() + ">" : "";
        const string args_string = is_argumentless_ ? "" : "(" + join(args_, ", ") + ")";
        return name_ + template_type_string + args_string;
    }
}
