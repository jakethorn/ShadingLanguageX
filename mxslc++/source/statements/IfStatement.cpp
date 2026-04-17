//
// Created by jaket on 10/04/2026.
//

#include "IfStatement.h"

#include "expressions/Expression.h"
#include "runtime/Runtime.h"
#include "runtime/TypeInfo.h"
#include "runtime/Variable2.h"
#include "utils/instantiate_template_types_utils.h"

IfStatement::IfStatement(Token token, ExprPtr cond_expr, StmtPtr then_body, StmtPtr else_body)
    : Statement{std::move(token)}, cond_expr_{std::move(cond_expr)}, then_body_{std::move(then_body)}, else_body_{std::move(else_body)} { }

StmtPtr IfStatement::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    ExprPtr cond_expr = ::instantiate_template_types(cond_expr_, template_type);
    StmtPtr then_body = then_body_->instantiate_template_types(template_type);
    StmtPtr else_body = else_body_->instantiate_template_types(template_type);
    return std::make_unique<IfStatement>(token_, std::move(cond_expr), std::move(then_body), std::move(else_body));
}

void IfStatement::execute_impl() const
{
    cond_expr_->init(TypeInfo::Bool);
    const VarPtr2 cond = cond_expr_->evaluate();

    runtime().enter_inline_scope();
    if (cond->value_as<bool>())
    {
        then_body_->execute();
    }
    else if (else_body_)
    {
        else_body_->execute();
    }
    runtime().exit_scope();
}
