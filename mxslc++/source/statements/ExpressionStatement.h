//
// Created by jaket on 08/04/2026.
//

#ifndef MXSLC_EXPRESSIONSTATEMENT_H
#define MXSLC_EXPRESSIONSTATEMENT_H

#include "utils/common.h"
#include "Statement.h"

class ExpressionStatement final : public Statement
{
public:
    explicit ExpressionStatement(ExprPtr expr);
    ~ExpressionStatement() override;

    StmtPtr instantiate_template_types(const TypeInfoPtr& template_type) const override;

protected:
    void execute_impl() const override;

private:
    ExprPtr expr_;
};

#endif //MXSLC_EXPRESSIONSTATEMENT_H
