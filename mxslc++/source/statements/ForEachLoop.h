//
// Created by jaket on 30/03/2026.
//

#ifndef MXSLC_FOREACHLOOP_H
#define MXSLC_FOREACHLOOP_H

#include "Statement.h"
#include "Token.h"

class ForEachLoop final : public Statement
{
public:
    ForEachLoop(const Runtime& runtime, TypeInfoPtr type, Token name, ExprPtr iter_expr, StmtPtr body);

    [[nodiscard]] StmtPtr instantiate_template_types(const TypeInfoPtr& template_type) const override;
    void execute() const override;

private:
    TypeInfoPtr type_;
    Token name_;
    ExprPtr iter_expr_;
    StmtPtr body_;
};

#endif //MXSLC_FOREACHLOOP_H
