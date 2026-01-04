//
// Created by jaket on 04/01/2026.
//

#ifndef FENNEC_RETURNSTATEMENT_H
#define FENNEC_RETURNSTATEMENT_H

#include "Statement.h"

class ReturnStatement final : public Statement
{
public:
    ReturnStatement(const Runtime& runtime, ExprPtr expr);

    StmtPtr instantiate_templated_types(const Type& template_type) const override;
    void init() override;
    void execute() override;

private:
    ExprPtr expr_;
};

#endif //FENNEC_RETURNSTATEMENT_H
