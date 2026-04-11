//
// Created by jaket on 30/03/2026.
//

#ifndef MXSLC_FORLOOP_H
#define MXSLC_FORLOOP_H

#include "Statement.h"
#include "Token.h"

class ForRangeLoop final : public Statement
{
public:
    ForRangeLoop(const Runtime& runtime, Token token, TypeInfoPtr type, string name, ExprPtr lower_expr, ExprPtr step_expr, ExprPtr upper_expr, StmtPtr body);

    StmtPtr instantiate_template_types(const TypeInfoPtr& template_type) const override;

protected:
    void execute_impl() const override;

private:
    TypeInfoPtr type_;
    string name_;
    ExprPtr lower_expr_;
    ExprPtr step_expr_;
    ExprPtr upper_expr_;
    StmtPtr body_;
};

#endif //MXSLC_FORLOOP_H
