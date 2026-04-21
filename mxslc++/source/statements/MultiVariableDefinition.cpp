//
// Created by jaket on 09/01/2026.
//

#include "MultiVariableDefinition.h"
#include "expressions/Expression.h"
#include "runtime/Runtime.h"
#include "runtime/TypeInfo.h"
#include "runtime/Scope.h"
#include "runtime/Variable2.h"

MultiVariableDefinition::MultiVariableDefinition(Token token, TypeInfoPtr type, ExprPtr expr)
    : Statement{std::move(token)}, type_{std::move(type)}, expr_{std::move(expr)}
{

}

StmtPtr MultiVariableDefinition::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    TypeInfoPtr type = type_->instantiate_template_types(template_type);
    ExprPtr expr = expr_->instantiate_template_types(template_type);
    return std::make_unique<MultiVariableDefinition>(token_, std::move(type), std::move(expr));
}

void MultiVariableDefinition::execute_impl() const
{
    const TypeInfoPtr type = runtime().scope().resolve_type(type_);
    expr_->init(type);

    const VarPtr2 value = expr_->evaluate();
    for (size_t i = 0; i < value->child_count(); ++i)
    {
        VarPtr2 child_value = value->child(i);
        VarPtr2 var = std::make_shared<Variable2>(type->field(i).modifiers(), type->field_type(i), child_value);
        runtime().scope().add_variable(type->field_name(i), std::move(var));
    }
}
