//
// Created by jaket on 08/04/2026.
//

//
// Created by jaket on 28/11/2025.
//

#include "ExpressionStatement.h"

#include "expressions/Expression.h"

ExpressionStatement::ExpressionStatement(ExprPtr expr)
    : Statement{expr->token()}, expr_{std::move(expr)}
{

}

ExpressionStatement::~ExpressionStatement() = default;

StmtPtr ExpressionStatement::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    ExprPtr expr = expr_->instantiate_template_types(template_type);
    return std::make_unique<ExpressionStatement>(std::move(expr));
}

void ExpressionStatement::execute_impl() const
{
    expr_->init();
    VarPtr2 _ = expr_->evaluate();
}
