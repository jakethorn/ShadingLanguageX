//
// Created by jaket on 27/03/2026.
//

#include "VariableAssignment.h"

#include "expressions/Expression.h"
#include "runtime/Variable.h"

VariableAssignment::VariableAssignment(Token token, ExprPtr lhs_expr, ExprPtr rhs_expr)
    : Statement{std::move(token)}, lhs_expr_{std::move(lhs_expr)}, rhs_expr_{std::move(rhs_expr)}
{

}

VariableAssignment::~VariableAssignment() = default;

StmtPtr VariableAssignment::instantiate_template_types(const TypePtr& template_type) const
{
    ExprPtr lhs = lhs_expr_->instantiate_template_types(template_type);
    ExprPtr rhs = rhs_expr_->instantiate_template_types(template_type);
    return std::make_unique<VariableAssignment>(token_, std::move(lhs), std::move(rhs));
}

void VariableAssignment::execute_impl() const
{
    lhs_expr_->init();
    rhs_expr_->init(lhs_expr_->type());
    const VarPtr lhs = lhs_expr_->evaluate();
    const VarPtr rhs = rhs_expr_->evaluate();
    lhs->copy_value(rhs);
}
