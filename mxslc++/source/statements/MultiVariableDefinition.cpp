//
// Created by jaket on 09/01/2026.
//

#include "MultiVariableDefinition.h"
#include "expressions/Expression.h"
#include "runtime/Runtime.h"
#include "runtime/TypeInfo.h"
#include "runtime/Variable.h"
#include "values/Value.h"

MultiVariableDefinition::MultiVariableDefinition(const Runtime& runtime, Token token, TypeInfoPtr type, ExprPtr expr)
    : Statement{runtime, std::move(token)}, type_{std::move(type)}, expr_{std::move(expr)}
{

}

StmtPtr MultiVariableDefinition::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    TypeInfoPtr type = type_->instantiate_template_types(template_type);
    ExprPtr expr = expr_->instantiate_template_types(template_type);
    return std::make_unique<MultiVariableDefinition>(runtime_, token_, std::move(type), std::move(expr));
}

void MultiVariableDefinition::execute_impl() const
{
    const TypeInfoPtr type = runtime_.scope().resolve_type(type_);
    expr_->init(type);

    const ValuePtr value = expr_->evaluate();
    for (size_t i = 0; i < type->field_count(); ++i)
    {
        ValuePtr subvalue = value->subvalue(i);
        VarPtr var = std::make_shared<Variable>(type->field(i).modifiers(), type->field_name(i), std::move(subvalue));
        runtime_.scope().add_variable(std::move(var));
    }
}
