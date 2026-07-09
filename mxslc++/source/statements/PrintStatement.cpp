//
// Created by jaket on 01/01/2026.
//

#include <iostream>
#include "statements/PrintStatement.h"
#include "expressions/Expression.h"
#include "runtime/Variable.h"
#include "runtime/utils/monomorphize.h"
#include "statements/interface.h"
#include "values/Value.h"

namespace mxslc::statements
{
    PrintStatement::PrintStatement(Token token, vector<ExprPtr> exprs) : Statement{std::move(token)}, exprs_{std::move(exprs)}
    {

    }

    StmtPtr PrintStatement::monomorphize(const TypePtr& template_type) const
    {
        vector<ExprPtr> exprs = template_utils::monomorphize(exprs_, template_type);
        return create_statement<PrintStatement>(token_, std::move(exprs));
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
}
