//
// Created by jaket on 28/11/2025.
//

#include "VariableDefinition.h"

#include "runtime/Scope.h"
#include "expressions/Expression.h"
#include "runtime/Runtime.h"
#include "runtime/Variable.h"
#include "values/Value.h"
#include "values/ValueFactory.h"

VariableDefinition::VariableDefinition(ModifierList mods, TypeInfoPtr type, Token name, ExprPtr expr)
    : Statement{std::move(name)}, mods_{std::move(mods)}, type_{std::move(type)}, expr_{std::move(expr)}
{

}

VariableDefinition::~VariableDefinition() = default;

StmtPtr VariableDefinition::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    TypeInfoPtr type = type_->instantiate_template_types(template_type);
    ExprPtr expr = expr_ ? expr_->instantiate_template_types(template_type) : nullptr;
    return std::make_unique<VariableDefinition>(mods_, std::move(type), token_, std::move(expr));
}

void VariableDefinition::execute_impl() const
{
    const TypeInfoPtr type = runtime().scope().resolve_type(type_);

    VarPtr value;
    if (expr_)
    {
        expr_->init(type);
        value = expr_->evaluate();
    }
    else
    {
        value = ValueFactory::create_default_value(type);
    }

    VarPtr var = Variable::create(mods_, type, value);
    runtime().scope().add_variable(name(), std::move(var));
}
