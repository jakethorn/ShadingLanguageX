//
// Created by jaket on 16/04/2026.
//

#ifndef MXSLC_FUNCTIONDEFINITION2_H
#define MXSLC_FUNCTIONDEFINITION2_H

#include "Statement.h"
#include "runtime/ModifierList.h"
#include "runtime/ParameterList.h"

class FunctionDefinition2 final : public Statement
{
public:
    FunctionDefinition2(ModifierList mods, TypeInfoPtr type, Token name, vector<TypeInfoPtr> template_types, ParameterList params, StmtPtr body, ExprPtr return_expr);

    StmtPtr instantiate_template_types(const TypeInfoPtr& template_type) const override;

protected:
    void execute_impl() const override;

private:
    const string& name() const { return token_.lexeme(); }
    bool is_templated() const { return not template_types_.empty(); }

    static void write_function_definition(const FuncPtr2& func);

    ModifierList mods_;
    TypeInfoPtr type_;
    vector<TypeInfoPtr> template_types_;
    ParameterList params_;
    StmtPtr body_;
    ExprPtr return_expr_;

    vector<FuncPtr2> funcs_;
};

#endif //MXSLC_FUNCTIONDEFINITION2_H
