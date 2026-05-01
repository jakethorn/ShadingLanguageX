//
// Created by jaket on 09/01/2026.
//

#include "MultiVariableDefinition.h"
#include "expressions/Expression.h"
#include "runtime/Runtime.h"
#include "runtime/Type.h"
#include "runtime/Scope.h"
#include "runtime/Variable.h"

MultiVariableDefinition::MultiVariableDefinition(Token token, TypePtr type, ExprPtr expr)
    : Statement{std::move(token)}, type_{std::move(type)}, expr_{std::move(expr)}
{

}

StmtPtr MultiVariableDefinition::instantiate_template_types(const TypePtr& template_type) const
{
    TypePtr type = type_->instantiate_template_types(template_type);
    ExprPtr expr = expr_->instantiate_template_types(template_type);
    return std::make_unique<MultiVariableDefinition>(token_, std::move(type), std::move(expr));
}

void MultiVariableDefinition::execute_impl() const
{
    const TypePtr type = runtime().scope().resolve_type(type_);
    expr_->init(type);

    const VarPtr value = expr_->evaluate();
    for (size_t i = 0; i < value->child_count(); ++i)
    {
        VarPtr child_value = value->child(i);
        VarPtr var = Variable::create(type->field(i).modifiers(), type->field_type(i), child_value);
        runtime().scope().add_variable(type->field_name(i), std::move(var));
    }
}
