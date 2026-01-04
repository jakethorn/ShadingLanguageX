//
// Created by jaket on 04/01/2026.
//

#include "ReturnStatement.h"
#include "expressions/Expression.h"

ReturnStatement::ReturnStatement(const Runtime& runtime, ExprPtr expr) : Statement{runtime}, expr_{std::move(expr)}
{

}

StmtPtr ReturnStatement::instantiate_templated_types(const Type& template_type) const
{
    ExprPtr expr = expr_->instantiate_templated_types(template_type);
    return std::make_unique<ReturnStatement>(runtime_, std::move(expr));
}

void ReturnStatement::init()
{

}

void ReturnStatement::execute()
{

}
