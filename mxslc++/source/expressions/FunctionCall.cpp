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

    namespace
    {
        // Operators are parsed into calls to their dunder functions (see ExpressionFactory), the operator token is
        // kept as the call's token, which is used here to print the call as an operator again.

        struct Operator
        {
            string symbol;
            Precedence precedence;
        };

        const unordered_map<string, Operator>& binary_operators()
        {
            static const unordered_map<string, Operator> operators {
                {"__add__", {"+", Precedence::Term}},
                {"__sub__", {"-", Precedence::Term}},
                {"__mul__", {"*", Precedence::Factor}},
                {"__div__", {"/", Precedence::Factor}},
                {"__mod__", {"%", Precedence::Factor}},
                {"__pow__", {"^", Precedence::Exponent}},
                {"__eq__", {"==", Precedence::Equality}},
                {"__ne__", {"!=", Precedence::Equality}},
                {"__gt__", {">", Precedence::Relational}},
                {"__lt__", {"<", Precedence::Relational}},
                {"__ge__", {">=", Precedence::Relational}},
                {"__le__", {"<=", Precedence::Relational}},
                {"__and__", {"&", Precedence::Logical}},
                {"__or__", {"|", Precedence::Logical}},
            };
            return operators;
        }

        const unordered_map<string, string>& unary_operators()
        {
            static const unordered_map<string, string> operators {
                {"__pos__", "+"},
                {"__neg__", "-"},
                {"__not__", "!"},
            };
            return operators;
        }

        Precedence next(const Precedence precedence)
        {
            return static_cast<Precedence>(static_cast<int>(precedence) + 1);
        }

        bool has_plain_arguments(const ArgumentList& args, const size_t count)
        {
            if (args.size() != count)
                return false;
            for (const Argument& arg : args)
                if (arg.has_name() or not arg.modifiers().empty() or arg.expression() == nullptr)
                    return false;
            return true;
        }

        bool is_binary_operator(const string& name, const Token& token, const ArgumentList& args)
        {
            const auto it = binary_operators().find(name);
            return it != binary_operators().end() and token.lexeme() == it->second.symbol and has_plain_arguments(args, 2);
        }

        bool is_unary_operator(const string& name, const Token& token, const ArgumentList& args)
        {
            const auto it = unary_operators().find(name);
            return it != unary_operators().end() and token.lexeme() == it->second and has_plain_arguments(args, 1);
        }

        bool is_absolute_operator(const string& name, const Token& token, const ArgumentList& args)
        {
            return name == "__abs__" and token.lexeme() == "|" and has_plain_arguments(args, 1);
        }

        // __lt_le__ -> {"<", "<="}
        optional<std::pair<string, string>> ternary_relational_symbols(const string& name, const Token& token, const ArgumentList& args)
        {
            static const unordered_map<string, string> symbols {
                {"eq", "=="}, {"ne", "!="}, {"gt", ">"}, {"lt", "<"}, {"ge", ">="}, {"le", "<="}
            };

            if (name.size() != 9 or name.rfind("__", 0) != 0 or name[4] != '_' or name.substr(7) != "__")
                return std::nullopt;

            const auto op1 = symbols.find(name.substr(2, 2));
            const auto op2 = symbols.find(name.substr(5, 2));
            if (op1 == symbols.end() or op2 == symbols.end())
                return std::nullopt;
            if (token.lexeme() != op1->second or not has_plain_arguments(args, 3))
                return std::nullopt;

            return std::pair{op1->second, op2->second};
        }
    }

    string FunctionCall::to_string() const
    {
        if (is_binary_operator(name_, token_, args_))
        {
            const Operator& op = binary_operators().at(name_);
            // relational operators do not chain, `a < b < c` is a ternary relational expression
            const Precedence lhs_precedence = op.precedence == Precedence::Relational ? next(op.precedence) : op.precedence;
            const string lhs = args_[0].expression()->to_operand_string(lhs_precedence);
            const string rhs = args_[1].expression()->to_operand_string(next(op.precedence));
            return lhs + " " + op.symbol + " " + rhs;
        }

        if (is_unary_operator(name_, token_, args_))
            return unary_operators().at(name_) + args_[0].expression()->to_operand_string(Precedence::Compound);

        if (is_absolute_operator(name_, token_, args_))
            return "|" + args_[0].expression()->to_operand_string(next(Precedence::Logical)) + "|";

        if (const auto symbols = ternary_relational_symbols(name_, token_, args_))
        {
            const string lhs = args_[0].expression()->to_operand_string(Precedence::Range);
            const string middle = args_[1].expression()->to_operand_string(Precedence::Range);
            const string rhs = args_[2].expression()->to_operand_string(Precedence::Range);
            return lhs + " " + symbols->first + " " + middle + " " + symbols->second + " " + rhs;
        }

        const string template_type_string = template_type_ ? "<" + template_type_->to_string() + ">" : "";
        const string args_string = is_argumentless_ ? "" : "(" + join(args_, ", ") + ")";
        return name_ + template_type_string + args_string;
    }

    Precedence FunctionCall::precedence() const
    {
        if (is_binary_operator(name_, token_, args_))
            return binary_operators().at(name_).precedence;
        if (is_unary_operator(name_, token_, args_))
            return Precedence::Unary;
        if (ternary_relational_symbols(name_, token_, args_))
            return Precedence::Relational;
        return Precedence::Primary;
    }
}
