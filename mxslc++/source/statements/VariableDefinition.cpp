//
// Created by jaket on 28/11/2025.
//

#include "VariableDefinition.h"

#include "runtime/Scope.h"
#include "runtime/Variable.h"
#include "expressions/Expression.h"
#include "runtime/Runtime.h"
#include "values/Value.h"

VariableDefinition::VariableDefinition(const Runtime& runtime, Token token, ModifierList mods, TypeInfoPtr type, string name, ExprPtr expr)
    : Statement{runtime, std::move(token)}, mods_{std::move(mods)}, type_{std::move(type)}, name_{std::move(name)}, expr_{std::move(expr)}
{

}

VariableDefinition::~VariableDefinition() = default;

StmtPtr VariableDefinition::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    TypeInfoPtr type = type_->instantiate_template_types(template_type);
    ExprPtr expr = expr_->instantiate_template_types(template_type);
    return std::make_unique<VariableDefinition>(runtime_, token_, mods_, std::move(type), name_, std::move(expr));
}

void VariableDefinition::execute_impl() const
{
    Scope& scope = runtime_.scope();

    const TypeInfoPtr type = scope.resolve_type(type_);
    expr_->init(type);
    ValuePtr val = expr_->evaluate();
    val->set_name(name_);
    VarPtr var = std::make_shared<Variable>(mods_, name_, std::move(val));
    scope.add_variable(std::move(var));
}
