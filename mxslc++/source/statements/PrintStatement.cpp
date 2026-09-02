//
// Created by jaket on 01/01/2026.
//

#include <iostream>

#include "statements/PrintStatement.h"

#include "expressions/Expression.h"
#include "runtime/variables/Variable.h"
#include "runtime/utils/monomorphize.h"
#include "statements/interface.h"
#include "serialize/values/Value.h"

namespace mxslc::statements
{
    PrintStatement::PrintStatement(vector<ExprPtr> exprs, Token token) : Statement{std::move(token)}, exprs_{std::move(exprs)}
    {

    }

    StmtPtr PrintStatement::monomorphize(const TypePtr& template_type) const
    {
        vector<ExprPtr> exprs = runtime_utils::monomorphize(exprs_, template_type);
        return create_statement<PrintStatement>(std::move(exprs), token_);
    }

    void PrintStatement::execute_impl() const
    {
        std::cout << '\n';
        for (const ExprPtr& expr : exprs_)
        {
            expr->init();
            const VarPtr var = expr->evaluate();
            std::cout << var->to_string() << '\n';
        }
    }

    string PrintStatement::to_string() const
    {
        return "print " + join(exprs_, ", ") + ";";
    }
}
