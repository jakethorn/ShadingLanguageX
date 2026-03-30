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
    ForRangeLoop(const Runtime& runtime, TypeInfoPtr type, Token name, ExprPtr lower_expr, ExprPtr step_expr, ExprPtr upper_expr, vector<StmtPtr> body);

    [[nodiscard]] StmtPtr instantiate_template_types(const TypeInfoPtr& template_type) const override;
    void execute() const override;

private:
    TypeInfoPtr type_;
    Token name_;
    ExprPtr lower_expr_;
    ExprPtr step_expr_;
    ExprPtr upper_expr_;
    vector<StmtPtr> body_;
};

#endif //MXSLC_FORLOOP_H
