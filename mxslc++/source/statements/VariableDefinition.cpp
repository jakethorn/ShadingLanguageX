//
// Created by jaket on 28/11/2025.
//

#include "statements/VariableDefinition.h"

#include "runtime/Scope.h"
#include "expressions/Expression.h"
#include "expressions/RuntimeExpression.h"
#include "runtime/Runtime.h"
#include "runtime/Variable.h"
#include "values/ValueFactory.h"

namespace mxslc
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

void VariableDefinition::set_attributes(AttributeList attrs)
{
    if (expr_)
        expr_->set_attributes(std::move(attrs));
}

StmtPtr VariableDefinition::instantiate_template_types(const TypePtr& template_type) const
{
    TypePtr type = type_->instantiate_template_types(template_type);
    ExprPtr expr = expr_ ? expr_->instantiate_template_types(template_type) : nullptr;
    return std::make_unique<VariableDefinition>(mods_, std::move(type), name_, std::move(expr), token_);
}

TypePtr VariableDefinition::type() const
{
    return scope().resolve_type(type_);
}

const string& VariableDefinition::name() const
{
    return name_;
}

void VariableDefinition::execute_impl() const
{
    TypePtr type = scope().resolve_type(type_);
    VarPtr value;

    if (mods_.contains(TokenType::Global))
    {
        if (VarPtr global = runtime().global(name_))
        {
            const ExprPtr value_expr = std::make_shared<RuntimeExpression>(std::move(global));
            value_expr->init(type);
            value = value_expr->evaluate();
        }
    }

    if (value == nullptr)
    {
        if (expr_)
        {
            expr_->init(type);
            value = expr_->evaluate();
        }
        else
        {
            value = ValueFactory::create_default_value(type);
        }
    }

    const VarPtr var = Variable::create(mods_, std::move(type), value);
    var->add_to_scope(name_);
}
}

