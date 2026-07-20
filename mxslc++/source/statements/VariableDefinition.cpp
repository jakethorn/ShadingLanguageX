//
// Created by jaket on 28/11/2025.
//

#include "statements/VariableDefinition.h"

#include "runtime/Scope.h"
#include "expressions/Expression.h"
#include "expressions/interface.h"
#include "expressions/RuntimeExpression.h"
#include "runtime/interface.h"
#include "runtime/Runtime.h"
#include "runtime/Type.h"
#include "runtime/Variable.h"
#include "runtime/utils/invoke.h"
#include "runtime/utils/monomorphize.h"
#include "statements/interface.h"

namespace mxslc::statements
{
    VariableDefinition::VariableDefinition(ModifierList mods, TypePtr type, string name, ExprPtr expr)
        : VariableDefinition{std::move(mods), std::move(type), std::move(name), std::move(expr), Token{}}
    {

    }

    VariableDefinition::VariableDefinition(ModifierList mods, TypePtr type, string name, ExprPtr expr, Token token)
        : Statement{std::move(token)}, mods_{std::move(mods)}, type_{std::move(type)}, name_{std::move(name)}, expr_{std::move(expr)}
    {

    }

    VariableDefinition::~VariableDefinition() = default;

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
        return create_statement<VariableDefinition>(mods_, std::move(type), name_, std::move(expr), token_);
    }

    void VariableDefinition::init()
    {
        type_ = scope().resolve_type(type_);
    }

    void VariableDefinition::execute_impl() const
    {
        VarPtr value;

        if (mods_.contains(TokenType::Global))
            value = evaluate_global();
        else if (mods_.contains(TokenType::Geomprop))
            value = evaluate_geomprop();

        if (value == nullptr)
        {
            if (expr_)
            {
                expr_->init(type_);
                value = expr_->evaluate();
            }
            else
            {
                value = create_variable(type_);
            }
        }

        const VarPtr var = create_variable(mods_, type_, value);
        var->add_to_scope(name_);
    }

    VarPtr VariableDefinition::evaluate_global() const
    {
        if (VarPtr global = runtime().global(name_))
        {
            // init and evaluate here for type checking
            const ExprPtr value_expr = create_expression<RuntimeExpression>(std::move(global));
            value_expr->init(type_);
            return value_expr->evaluate();
        }

        return nullptr;
    }

    VarPtr VariableDefinition::evaluate_geomprop() const
    {
        ArgumentList args{name_};
        if (expr_)
            args.append(expr_);
        return runtime_utils::invoke_function(type_, "geompropvalue", std::move(args));
    }

    string VariableDefinition::to_string() const
    {
        string mods_string = mods_.to_string();
        if (not mods_string.empty())
            mods_string += " ";

        string result = mods_string;
        result += type_->to_string() + " " + name_;
        if (expr_)
            result += " = " + expr_->to_string();
        result += ";";

        return result;
    }
}
