//
// Created by jaket on 27/03/2026.
//

#include "VariableAssignment.h"

#include "expressions/Expression.h"

VariableAssignment::VariableAssignment(const Runtime& runtime, Token token, ExprPtr lhs, ExprPtr rhs)
    : Statement{runtime, std::move(token)}, lhs_{std::move(lhs)}, rhs_{std::move(rhs)}
{

}

VariableAssignment::~VariableAssignment() = default;

StmtPtr VariableAssignment::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    ExprPtr lhs = lhs_->instantiate_template_types(template_type);
    ExprPtr rhs = rhs_->instantiate_template_types(template_type);
    return std::make_unique<VariableAssignment>(runtime_, token_, std::move(lhs), std::move(rhs));
}

void VariableAssignment::execute_impl() const
{
    lhs_->init();
    rhs_->init(lhs_->type());
    const ValuePtr value = rhs_->evaluate();
    lhs_->assign(value);
}
