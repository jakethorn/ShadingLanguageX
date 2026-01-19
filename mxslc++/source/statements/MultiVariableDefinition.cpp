//
// Created by jaket on 09/01/2026.
//

#include "MultiVariableDefinition.h"
#include "expressions/Expression.h"
#include "runtime/Runtime.h"
#include "runtime/TypeInfo.h"
#include "values/Value.h"

MultiVariableDefinition::MultiVariableDefinition(const Runtime& runtime, TypeInfoPtr type, ExprPtr expr)
    : Statement{runtime}, type_{std::move(type)}, expr_{std::move(expr)}
{

}

StmtPtr MultiVariableDefinition::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    TypeInfoPtr type = type_->instantiate_template_types(template_type);
    ExprPtr expr = expr_->instantiate_template_types(template_type);
    return std::make_unique<MultiVariableDefinition>(runtime_, std::move(type), std::move(expr));
}

void MultiVariableDefinition::execute() const
{
    const TypeInfoPtr type = runtime_.scope().get_type(type_);
    expr_->init(type);

    const ValuePtr value = expr_->evaluate();
    for (size_t i = 0; i < type->field_count(); ++i)
    {
        ValuePtr subvalue = value->subvalue(i);
        Variable var{type->field(i).modifiers(), type->field(i).type(), type->field(i).name_token(), std::move(subvalue)};
        runtime_.scope().add_variable(std::move(var));
    }
}
