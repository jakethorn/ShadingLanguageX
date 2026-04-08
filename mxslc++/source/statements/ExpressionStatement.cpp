//
// Created by jaket on 08/04/2026.
//

//
// Created by jaket on 28/11/2025.
//

#include "ExpressionStatement.h"

#include "runtime/Scope.h"
#include "expressions/Expression.h"
#include "runtime/Runtime.h"
#include "values/Value.h"

ExpressionStatement::ExpressionStatement(const Runtime& runtime, ExprPtr expr)
    : Statement{runtime}, expr_{std::move(expr)}
{

}

ExpressionStatement::~ExpressionStatement() = default;

StmtPtr ExpressionStatement::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    ExprPtr expr = expr_->instantiate_template_types(template_type);
    return std::make_unique<ExpressionStatement>(runtime_, std::move(expr));
}

void ExpressionStatement::execute() const
{
    expr_->init();
    ValuePtr _ = expr_->evaluate();
}
