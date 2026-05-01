//
// Created by jaket on 09/01/2026.
//

#ifndef MXSLC_MULTIVARIABLEDEFINITION_H
#define MXSLC_MULTIVARIABLEDEFINITION_H

#include "Statement.h"
#include "Token.h"

class MultiVariableDefinition final : public Statement
{
public:
    MultiVariableDefinition(Token token, TypePtr type, ExprPtr expr);

    StmtPtr instantiate_template_types(const TypePtr& template_type) const override;

protected:
    void execute_impl() const override;

private:
    TypePtr type_;
    ExprPtr expr_;
};

#endif //MXSLC_MULTIVARIABLEDEFINITION_H
