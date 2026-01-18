//
// Created by jaket on 01/01/2026.
//

#include <iostream>
#include "PrintStatement.h"
#include "expressions/Expression.h"
#include "utils/instantiate_template_types_utils.h"
#include "values/Value.h"

PrintStatement::PrintStatement(const Runtime& runtime, vector<ExprPtr> exprs) : Statement{runtime}, exprs_{std::move(exprs)}
{

}

StmtPtr PrintStatement::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    vector<ExprPtr> exprs = ::instantiate_template_types(exprs_, template_type);
    return std::make_unique<PrintStatement>(runtime_, std::move(exprs));
}

void PrintStatement::execute() const
{
    std::cout << std::endl;
    for (const ExprPtr& expr : exprs_)
    {
        expr->init();
        const ValuePtr value = expr->evaluate();
        std::cout << value->str() << std::endl;
    }
}
