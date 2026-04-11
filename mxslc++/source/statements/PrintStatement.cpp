//
// Created by jaket on 01/01/2026.
//

#include <iostream>
#include "PrintStatement.h"
#include "expressions/Expression.h"
#include "utils/instantiate_template_types_utils.h"
#include "values/Value.h"

PrintStatement::PrintStatement(const Runtime& runtime, Token token, vector<ExprPtr> exprs) : Statement{runtime, std::move(token)}, exprs_{std::move(exprs)}
{

}

StmtPtr PrintStatement::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    vector<ExprPtr> exprs = ::instantiate_template_types(exprs_, template_type);
    return std::make_unique<PrintStatement>(runtime_, token_, std::move(exprs));
}

void PrintStatement::execute_impl() const
{
    std::cout << std::endl;
    for (const ExprPtr& expr : exprs_)
    {
        expr->init();
        const ValuePtr value = expr->evaluate();
        std::cout << value->str() << std::endl;
    }
}
