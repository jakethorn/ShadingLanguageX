//
// Created by jaket on 07/11/2025.
//

#ifndef FENNEC_VARIABLEDEFINITION_H
#define FENNEC_VARIABLEDEFINITION_H

#include "utils/common.h"
#include "Statement.h"
#include "Token.h"
#include "runtime/Type.h"

class VariableDefinition final : public Statement
{
public:
    VariableDefinition(const Runtime& runtime, vector<string> modifiers, Type type, Token name, ExprPtr expr);
    ~VariableDefinition() override;

    StmtPtr instantiate_templated_types(const Type& template_type) const override;
    void init() override;
    void execute() override;

private:
    vector<string> modifiers_;
    Type type_;
    Token name_;
    ExprPtr expr_;
};

#endif //FENNEC_VARIABLEDEFINITION_H
