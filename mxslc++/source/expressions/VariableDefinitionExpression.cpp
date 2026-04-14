//
// Created by jaket on 13/04/2026.
//

#include "VariableDefinitionExpression.h"

#include "Identifier.h"
#include "runtime/Variable.h"
#include "statements/VariableDefinition.h"

VariableDefinitionExpression::VariableDefinitionExpression(const Runtime& runtime, ModifierList mods, TypeInfoPtr type, Token name)
    : VariableDefinitionExpression{
        runtime,
        std::make_unique<VariableDefinition>(runtime, mods.without(TokenType::Out), std::move(type), name, nullptr),
        std::make_unique<Identifier>(runtime, name)
    }
{

}

VariableDefinitionExpression::VariableDefinitionExpression(const Runtime& runtime, StmtPtr var_def, ExprPtr identifier)
    : Expression{runtime, var_def->token()}, var_def_{std::move(var_def)}, identifier_{std::move(identifier)}
{

}

ExprPtr VariableDefinitionExpression::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    StmtPtr var_def = var_def_->instantiate_template_types(template_type);
    ExprPtr identifier = identifier_->instantiate_template_types(template_type);
    return std::make_unique<VariableDefinitionExpression>(runtime_, std::move(var_def), std::move(identifier));
}

void VariableDefinitionExpression::init_impl(const vector<TypeInfoPtr>& types)
{
    var_def_->execute();
    identifier_->init(types);
}

TypeInfoPtr VariableDefinitionExpression::type_impl() const
{
    return identifier_->type();
}

ValuePtr VariableDefinitionExpression::evaluate_impl() const
{
    return identifier_->evaluate();
}

void VariableDefinitionExpression::assign_impl(const ValuePtr& value)
{
    identifier_->assign(value);
}
