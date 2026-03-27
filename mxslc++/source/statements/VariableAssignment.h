//
// Created by jaket on 27/03/2026.
//

#ifndef MXSLC_VARIABLEASSIGNMENT_H
#define MXSLC_VARIABLEASSIGNMENT_H

#include "Statement.h"
#include "Token.h"

class VariableAssignment final : public Statement
{
public:
    VariableAssignment(const Runtime& runtime, ExprPtr lhs, ExprPtr rhs);
    ~VariableAssignment() override;

    [[nodiscard]] StmtPtr instantiate_template_types(const TypeInfoPtr& template_type) const override;
    void execute() const override;

private:
    ExprPtr lhs_;
    ExprPtr rhs_;
};

#endif //MXSLC_VARIABLEASSIGNMENT_H
