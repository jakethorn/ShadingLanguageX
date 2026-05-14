//
// Created by jaket on 16/04/2026.
//

#ifndef MXSLC_FUNCTIONDEFINITION_H
#define MXSLC_FUNCTIONDEFINITION_H

#include "Statement.h"
#include "runtime/ModifierList.h"
#include "runtime/ParameterList.h"

class FunctionDefinition final : public Statement
{
public:
    FunctionDefinition(ModifierList mods, TypePtr type, string name, vector<TypePtr> template_types, ParameterList params, StmtPtr body, ExprPtr return_expr);
    FunctionDefinition(ModifierList mods, TypePtr type, string name, vector<TypePtr> template_types, ParameterList params, StmtPtr body, ExprPtr return_expr, Token token);

    void set_attributes(AttributeList attrs) override;

    StmtPtr instantiate_template_types(const TypePtr& template_type) const override;

    const vector<FuncPtr>& functions() const { return funcs_; }

protected:
    void execute_impl() const override;

private:
    bool is_templated() const { return not template_types_.empty(); }

    ModifierList mods_;
    TypePtr type_;
    string name_;
    vector<TypePtr> template_types_;
    ParameterList params_;
    StmtPtr body_;
    ExprPtr return_expr_;

    vector<FuncPtr> funcs_;

    AttributeList attrs_;
};

#endif //MXSLC_FUNCTIONDEFINITION_H
