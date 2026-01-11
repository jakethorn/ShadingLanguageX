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
    VariableDefinition(const Runtime& runtime, vector<string> mods, Type type, Token name, ExprPtr expr);
    ~VariableDefinition() override;

    [[nodiscard]] StmtPtr instantiate_template_types(const Type& template_type) const override;
    void execute() const override;

private:
    vector<string> mods_;
    Type type_;
    Token name_;
    ExprPtr expr_;
};

#endif //FENNEC_VARIABLEDEFINITION_H
