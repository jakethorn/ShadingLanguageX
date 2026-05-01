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

    StmtPtr instantiate_template_types(const TypePtr& template_type) const override;

protected:
    void execute_impl() const override;

private:
    const string& name() const { return token_.lexeme(); }
    bool is_templated() const { return not template_types_.empty(); }

    static void write_function_definition(const FuncPtr& func);

    ModifierList mods_;
    TypePtr type_;
    vector<TypePtr> template_types_;
    ParameterList params_;
    StmtPtr body_;
    ExprPtr return_expr_;

    vector<FuncPtr> funcs_;
};

#endif //MXSLC_FUNCTIONDEFINITION_H
