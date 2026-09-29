//
// Created by jaket on 28/11/2025.
//

#include "statements/VariableDefinition.h"

#include "runtime/Scope.h"
#include "expressions/Expression.h"
#include "expressions/Identifier.h"
#include "expressions/interface.h"
#include "expressions/RuntimeExpression.h"
#include "runtime/interface.h"
#include "runtime/Runtime.h"
#include "runtime/Type.h"
#include "runtime/variables/Variable.h"
#include "runtime/utils/invoke.h"
#include "runtime/utils/monomorphize.h"
#include "serialize/Serializer.h"
#include "statements/interface.h"

namespace mxslc::statements
{
    namespace
    {
        // the variable that is copied by a definition, e.g., fragCoord of `vec2 uv = fragCoord;`, unless its value is changed
        // by a global or geomprop
        IdentifierPtr copied_variable(const ModifierList& mods, const ExprPtr& expr)
        {
            if (mods.contains(TokenType::Global) or mods.contains(TokenType::Geomprop))
                return nullptr;
            return cast_expression<Identifier>(expr);
        }
    }

    VariableDefinition::VariableDefinition(ModifierList mods, TypePtr type, string name, ExprPtr expr)
        : VariableDefinition{std::move(mods), std::move(type), std::move(name), std::move(expr), Token{}}
    {

    }

    VariableDefinition::VariableDefinition(ModifierList mods, TypePtr type, string name, ExprPtr expr, Token token)
        : Statement{std::move(token)}, mods_{std::move(mods)}, type_{std::move(type)}, name_{std::move(name)}, expr_{std::move(expr)}, type_string_{type_ ? type_->to_string() : ""}
    {

    }

    VariableDefinition::~VariableDefinition() = default;

    const ModifierList &VariableDefinition::modifiers() const
    {
        return mods_;
    }

    TypePtr VariableDefinition::type() const
    {
        return scope().resolve_type(type_);
    }

    const string& VariableDefinition::name() const
    {
        return name_;
    }

    void VariableDefinition::set_attributes(AttributeList attrs)
    {
        if (expr_)
            expr_->set_attributes(std::move(attrs));
    }

    StmtPtr VariableDefinition::monomorphize(const TypePtr& template_type) const
    {
        auto&& [type, expr] = runtime_utils::monomorphize_all(template_type, type_, expr_);
        auto copy = create_statement<VariableDefinition>(mods_, std::move(type), name_, std::move(expr), token_);
        // the type is recorded by the decompile hints as it is written, e.g., T
        copy->type_string_ = type_string_;
        return copy;
    }

    void VariableDefinition::init()
    {
        type_ = scope().resolve_type(type_);
    }

    void VariableDefinition::execute_impl() const
    {
        const bool is_global = mods_.contains(TokenType::Global);
        const bool is_geomprop = mods_.contains(TokenType::Geomprop);

        VarPtr value;
        if (is_global)
        {
            value = evaluate_global();
        }

        if (not value and expr_)
        {
            expr_->init(type_);
            value = expr_->evaluate();
        }

        if (is_geomprop)
            value = evaluate_geomprop(value);

        if (not value)
            value = create_variable(type_);

        const VarPtr var = create_variable(mods_.without(TokenType::Global, TokenType::Geomprop), type_, value);
        var->add_to_scope(name_);
        serializer().hints().bind_variable(var, name_);
        if (const IdentifierPtr identifier = copied_variable(mods_, expr_))
            serializer().hints().copy_variable(var, identifier->variable());
    }

    StmtPtr VariableDefinition::with_expression(ExprPtr expr) const
    {
        return create_statement<VariableDefinition>(mods_, type_, name_, std::move(expr), token_);
    }

    string VariableDefinition::hint_skeleton() const
    {
        string mods_string = mods_.to_string();
        if (not mods_string.empty())
            mods_string += ' ';

        string result = mods_string + type_string_ + ' ' + name_;
        if (expr_)
        {
            // globals can be given a value by the compile options, so their value is recorded instead of their code, and
            // copies of variables are recorded as code, e.g., `vec2 uv = fragCoord;`
            const VarPtr var = scope().get_variable(name_);
            const bool is_global = mods_.contains(TokenType::Global);
            if (is_global)
                result += " = " + serialize::HintRecorder::value_skeleton(var);
            else if (copied_variable(mods_, expr_))
                result += " = " + expr_->to_string();
            else
                result += " = " + serialize::HintRecorder::expression_skeleton(expr_, var);
        }
        return result + ';';
    }

    VarPtr VariableDefinition::evaluate_global() const
    {
        if (VarPtr global = runtime().global(name_))
        {
            // init and evaluate here for type checking
            const ExprPtr value_expr = as_expression(std::move(global));
            value_expr->init(type_);
            return value_expr->evaluate();
        }

        return nullptr;
    }

    VarPtr VariableDefinition::evaluate_geomprop(VarPtr default_value) const
    {
        ArgumentList args{name_};
        if (default_value)
            args.add(std::move(default_value));
        return runtime_utils::invoke_function(type_, "geompropvalue", std::move(args));
    }

    string VariableDefinition::to_string() const
    {
        string mods_string = mods_.to_string();
        if (not mods_string.empty())
            mods_string += ' ';

        string result = mods_string;
        result += type_->to_string() + ' ' + name_;
        if (expr_)
            result += " = " + expr_->to_string();
        result += ';';

        return with_attributes(expr_ ? expr_->attributes() : AttributeList{}, result);
    }
}
