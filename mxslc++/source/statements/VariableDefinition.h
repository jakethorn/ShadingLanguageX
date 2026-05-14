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

    void set_attributes(AttributeList attrs) override;

    StmtPtr instantiate_template_types(const TypePtr& template_type) const override;

    const ModifierList& modifiers() const { return mods_; }
    TypePtr type() const;
    const string& name() const;

protected:
    void execute_impl() const override;

private:
    ModifierList mods_;
    TypePtr type_;
    ExprPtr expr_;
};

#endif //FENNEC_VARIABLEDEFINITION_H
