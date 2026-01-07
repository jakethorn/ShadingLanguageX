//
// Created by jaket on 04/01/2026.
//

#ifndef FENNEC_RETURNSTATEMENT_H
#define FENNEC_RETURNSTATEMENT_H

#include "Statement.h"
#include "runtime/Type.h"

class ReturnStatement final : public Statement
{
public:
    ReturnStatement(const Runtime& runtime, Type type, ExprPtr expr);

    [[nodiscard]] StmtPtr instantiate_templated_types(const Type& template_type) const override;
    void execute() override;

private:
    Type type_;
    ExprPtr expr_;
};

#endif //FENNEC_RETURNSTATEMENT_H
