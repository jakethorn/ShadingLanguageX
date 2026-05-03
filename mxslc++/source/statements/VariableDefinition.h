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
    VariableDefinition(ModifierList mods, TypePtr type, Token name, ExprPtr expr);
    ~VariableDefinition() override;

    StmtPtr instantiate_template_types(const TypePtr& template_type) const override;

protected:
    void execute_impl() const override;

private:
    const string& name() const { return token_.lexeme(); }

    ModifierList mods_;
    TypePtr type_;
    ExprPtr expr_;
};

#endif //FENNEC_VARIABLEDEFINITION_H
