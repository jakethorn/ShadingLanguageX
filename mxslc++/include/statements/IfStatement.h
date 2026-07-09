//
// Created by jaket on 10/04/2026.
//

#ifndef MXSLC_IFSTATEMENT_H
#define MXSLC_IFSTATEMENT_H

#include "statements/Statement.h"

namespace mxslc::statements
{
    class IfStatement final : public Statement
    {
    public:
        IfStatement(Token token, ExprPtr cond_expr, StmtPtr then_body, StmtPtr else_body);

        StmtPtr monomorphize(const TypePtr& template_type) const override;

    protected:
        void execute_impl() const override;

    private:
        ExprPtr cond_expr_;
        StmtPtr then_body_;
        StmtPtr else_body_;
    };
}

#endif //MXSLC_IFSTATEMENT_H
