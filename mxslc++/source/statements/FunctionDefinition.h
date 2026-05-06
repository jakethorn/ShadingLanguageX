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
    FunctionDefinition(ModifierList mods, TypePtr type, Token name, vector<TypePtr> template_types, ParameterList params, StmtPtr body, ExprPtr return_expr);

    void set_attributes(AttributeList attrs) override;

    StmtPtr instantiate_template_types(const TypePtr& template_type) const override;

    const vector<FuncPtr>& functions() const { return funcs_; }
    const AttributeList& attributes() const { return attrs_; }

protected:
    void execute_impl() const override;

private:
    const string& name() const { return token_.lexeme(); }
    bool is_templated() const { return not template_types_.empty(); }

    ModifierList mods_;
    TypePtr type_;
    vector<TypePtr> template_types_;
    ParameterList params_;
    StmtPtr body_;
    ExprPtr return_expr_;

    vector<FuncPtr> funcs_;

    AttributeList attrs_;
};

#endif //MXSLC_FUNCTIONDEFINITION_H
