//
// Created by jaket on 01/01/2026.
//

#ifndef FENNEC_PRINTSTATEMENT_H
#define FENNEC_PRINTSTATEMENT_H

#include "Statement.h"

class PrintStatement final : public Statement
{
public:
    PrintStatement(const Runtime& runtime, vector<ExprPtr> exprs);

    [[nodiscard]] StmtPtr instantiate_template_types(const Type& template_type) const override;
    void execute() const override;

private:
    vector<ExprPtr> exprs_;
};

#endif //FENNEC_PRINTSTATEMENT_H
