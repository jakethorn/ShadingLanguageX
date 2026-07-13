//
// Created by jaket on 10/04/2026.
//

#include "statements/IfStatement.h"

#include "expressions/Expression.h"
#include "runtime/Runtime.h"
#include "runtime/Type.h"
#include "runtime/Variable.h"
#include "runtime/utils/monomorphize.h"
#include "statements/interface.h"

namespace mxslc::statements
{
    IfStatement::IfStatement(Token token, ExprPtr cond_expr, StmtPtr then_body, StmtPtr else_body)
        : Statement{std::move(token)}, cond_expr_{std::move(cond_expr)}, then_body_{std::move(then_body)}, else_body_{std::move(else_body)} { }

    StmtPtr IfStatement::monomorphize(const TypePtr& template_type) const
    {
        auto&& [cond_expr, then_body, else_body] = runtime_utils::monomorphize_all(template_type, cond_expr_, then_body_, else_body_);
        return create_statement<IfStatement>(token_, std::move(cond_expr), std::move(then_body), std::move(else_body));
    }

    void IfStatement::execute_impl() const
    {
        if (not cond_expr_->try_init(Type::Bool))
            cond_expr_->init();

        const VarPtr cond = cond_expr_->evaluate();

        runtime().enter_scope();
        if (cond->basic())
        {
            then_body_->execute();
        }
        else if (else_body_)
        {
            else_body_->execute();
        }
        runtime().exit_scope();
    }
}
