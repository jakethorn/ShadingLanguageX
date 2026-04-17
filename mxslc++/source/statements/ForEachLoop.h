//
// Created by jaket on 30/03/2026.
//

#ifndef MXSLC_FOREACHLOOP_H
#define MXSLC_FOREACHLOOP_H

#include "Statement.h"
#include "Token.h"
#include "runtime/ModifierList.h"

class ForEachLoop final : public Statement
{
public:
    ForEachLoop(Token token, ModifierList mods, TypeInfoPtr type, string name, ExprPtr iter_expr, StmtPtr body);

    StmtPtr instantiate_template_types(const TypeInfoPtr& template_type) const override;

protected:
    void execute_impl() const override;

private:
    ModifierList mods_;
    TypeInfoPtr type_;
    string name_;
    ExprPtr iter_expr_;
    StmtPtr body_;
};

#endif //MXSLC_FOREACHLOOP_H
