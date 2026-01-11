//
// Created by jaket on 28/11/2025.
//

#include "VariableDefinition.h"

#include "runtime/Scope.h"
#include "runtime/Variable.h"
#include "expressions/Expression.h"
#include "runtime/Runtime.h"

VariableDefinition::VariableDefinition(const Runtime& runtime, vector<string> mods, Type type, Token name, ExprPtr expr)
    : Statement{runtime}, mods_{std::move(mods)}, type_{std::move(type)}, name_{std::move(name)}, expr_{std::move(expr)}
{

}

VariableDefinition::~VariableDefinition() = default;

StmtPtr VariableDefinition::instantiate_template_types(const Type& template_type) const
{
    Type type = type_.instantiate_template_types(template_type);
    ExprPtr expr = expr_->instantiate_template_types(template_type);
    return std::make_unique<VariableDefinition>(runtime_, mods_, std::move(type), name_, std::move(expr));
}

void VariableDefinition::execute() const
{
    expr_->init(type_);
    ValuePtr val = expr_->evaluate();
    Variable var{mods_, type_, name_, std::move(val)};
    runtime_.scope().add_variable(std::move(var));
}
