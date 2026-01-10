//
// Created by jaket on 09/01/2026.
//

#ifndef MXSLC_MULTIVARIABLEDEFINITION_H
#define MXSLC_MULTIVARIABLEDEFINITION_H

#include "Statement.h"
#include "Token.h"
#include "runtime/Type.h"
#include "runtime/Variable.h"

class MultiVariableDefinition final : public Statement
{
public:
    MultiVariableDefinition(const Runtime& runtime, vector<VariableDeclaration> decls, ExprPtr expr);

    [[nodiscard]] StmtPtr instantiate_template_types(const Type& template_type) const override;
    void execute() override;

private:
    vector<VariableDeclaration> decls_;
    ExprPtr expr_;
};

#endif //MXSLC_MULTIVARIABLEDEFINITION_H
