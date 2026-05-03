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
    VariableAssignment(Token token, ExprPtr lhs_expr, ExprPtr rhs_expr);
    ~VariableAssignment() override;

    StmtPtr instantiate_template_types(const TypePtr& template_type) const override;

protected:
    void execute_impl() const override;

private:
    ExprPtr lhs_expr_;
    ExprPtr rhs_expr_;
};

#endif //MXSLC_VARIABLEASSIGNMENT_H
