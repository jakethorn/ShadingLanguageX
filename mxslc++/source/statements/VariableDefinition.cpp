//
// Created by jaket on 28/11/2025.
//

#include "VariableDefinition.h"

#include "runtime/Scope.h"
#include "runtime/Variable.h"
#include "expressions/Expression.h"
#include "runtime/Runtime.h"

VariableDefinition::VariableDefinition(const Runtime& runtime, ModifierList mods, TypeInfoPtr type, Token name, ExprPtr expr)
    : Statement{runtime}, mods_{std::move(mods)}, type_{std::move(type)}, name_{std::move(name)}, expr_{std::move(expr)}
{

}

VariableDefinition::~VariableDefinition() = default;

StmtPtr VariableDefinition::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    TypeInfoPtr type = type_->instantiate_template_types(template_type);
    ExprPtr expr = expr_->instantiate_template_types(template_type);
    return std::make_unique<VariableDefinition>(runtime_, mods_, std::move(type), name_, std::move(expr));
}

void VariableDefinition::execute() const
{
    TypeInfoPtr type = runtime_.scope().get_type(type_);
    expr_->init(type);
    ValuePtr val = expr_->evaluate();
    Variable var{mods_, std::move(type), name_, std::move(val)};
    runtime_.scope().add_variable(std::move(var));
}
