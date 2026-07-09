//
// Created by jaket on 08/04/2026.
//

//
// Created by jaket on 28/11/2025.
//

#include "statements/ExpressionStatement.h"

#include "expressions/Expression.h"
#include "statements/interface.h"

namespace mxslc::statements
{
    ExpressionStatement::ExpressionStatement(ExprPtr expr)
        : Statement{expr->token()}, expr_{std::move(expr)}
    {

    }

    ExpressionStatement::~ExpressionStatement() = default;

    void ExpressionStatement::set_attributes(AttributeList attrs)
    {
        expr_->set_attributes(std::move(attrs));
    }

    StmtPtr ExpressionStatement::monomorphize(const TypePtr& template_type) const
    {
        ExprPtr expr = expr_->monomorphize(template_type);
        return create_statement<ExpressionStatement>(std::move(expr));
    }

    void ExpressionStatement::execute_impl() const
    {
        expr_->init();
        VarPtr _ = expr_->evaluate();
    }
}
