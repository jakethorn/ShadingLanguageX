//
// Created by jaket on 01/01/2026.
//

#include <iostream>
#include "PrintStatement.h"
#include "expressions/Expression.h"
#include "runtime/Variable.h"
#include "utils/instantiate_template_types_utils.h"
#include "values/Value.h"

PrintStatement::PrintStatement(Token token, vector<ExprPtr> exprs) : Statement{std::move(token)}, exprs_{std::move(exprs)}
{

}

StmtPtr PrintStatement::instantiate_template_types(const TypePtr& template_type) const
{
    vector<ExprPtr> exprs = ::instantiate_template_types(exprs_, template_type);
    return std::make_unique<PrintStatement>(token_, std::move(exprs));
}

void PrintStatement::execute_impl() const
{
    std::cout << std::endl;
    for (const ExprPtr& expr : exprs_)
    {
        expr->init();
        const VarPtr var = expr->evaluate();
        std::cout << var->str() << std::endl;
    }
}
