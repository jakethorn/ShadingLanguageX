//
// Created by jaket on 28/11/2025.
//

#include "VariableDefinition.h"

#include "runtime/Scope.h"
#include "runtime/Variable.h"
#include "expressions/Expression.h"
#include "runtime/Runtime.h"
#include "values/Value.h"
#include "values/ValueFactory.h"

VariableDefinition::VariableDefinition(const Runtime& runtime, ModifierList mods, TypeInfoPtr type, Token name, ExprPtr expr)
    : Statement{runtime, std::move(name)}, mods_{std::move(mods)}, type_{std::move(type)}, expr_{std::move(expr)}
{

}

VariableDefinition::~VariableDefinition() = default;

StmtPtr VariableDefinition::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    TypeInfoPtr type = type_->instantiate_template_types(template_type);
    ExprPtr expr = expr_ ? expr_->instantiate_template_types(template_type) : nullptr;
    return std::make_unique<VariableDefinition>(runtime_, mods_, std::move(type), token_, std::move(expr));
}

void VariableDefinition::execute_impl() const
{
    Scope& scope = runtime_.scope();

    const TypeInfoPtr type = scope.resolve_type(type_);

    ValuePtr val;
    if (expr_)
    {
        expr_->init(type);
        val = expr_->evaluate();
    }
    else
    {
        val = ValueFactory::create_default_value(type);
    }

    val->set_name(name());
    VarPtr var = std::make_shared<Variable>(mods_, name(), std::move(val));
    scope.add_variable(std::move(var));
}
