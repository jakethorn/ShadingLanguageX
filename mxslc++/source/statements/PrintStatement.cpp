//
// Created by jaket on 01/01/2026.
//

#include <iostream>
#include "PrintStatement.h"
#include "expressions/Expression.h"
#include "runtime/values/Value.h"

PrintStatement::PrintStatement(const Runtime& runtime, vector<ExprPtr> exprs) : Statement{runtime}, exprs_{std::move(exprs)}
{

}

StmtPtr PrintStatement::instantiate_templated_types(const Type& template_type) const
{
    vector<ExprPtr> instantiated_exprs;
    instantiated_exprs.reserve(exprs_.size());
    for (const ExprPtr& expr : exprs_)
        instantiated_exprs.push_back(expr->instantiate_templated_types(template_type));

    return std::make_unique<PrintStatement>(runtime_, std::move(instantiated_exprs));
}

void PrintStatement::execute()
{
    std::cout << std::endl;
    for (const ExprPtr& expr : exprs_)
    {
        expr->init();
        const ValuePtr value = expr->evaluate();
        std::cout << value->str() << std::endl;
    }
}
