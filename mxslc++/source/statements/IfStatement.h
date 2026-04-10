//
// Created by jaket on 10/04/2026.
//

#ifndef MXSLC_IFSTATEMENT_H
#define MXSLC_IFSTATEMENT_H

#include "Statement.h"

class IfStatement final : public Statement
{
public:
    IfStatement(const Runtime& runtime, ExprPtr cond_expr, StmtPtr then_body, StmtPtr else_body);

    [[nodiscard]] StmtPtr instantiate_template_types(const TypeInfoPtr& template_type) const override;
    void execute() const override;

private:
    ExprPtr cond_expr_;
    StmtPtr then_body_;
    StmtPtr else_body_;
};

#endif //MXSLC_IFSTATEMENT_H
