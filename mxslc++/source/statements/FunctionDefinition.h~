//
// Created by jaket on 03/01/2026.
//

#ifndef FENNEC_FUNCTIONDEFINITION_H
#define FENNEC_FUNCTIONDEFINITION_H

#include "utils/common.h"
#include "Statement.h"
#include "Token.h"
#include "runtime/ParameterList.h"

class FunctionDefinition final : public Statement
{
public:
    FunctionDefinition(const Runtime& runtime, vector<string> modifiers, Type type, Token name, vector<Type> template_types, ParameterList params, vector<StmtPtr> body, ExprPtr return_expr);

    [[nodiscard]] StmtPtr instantiate_template_types(const Type& template_type) const override;
    void execute() const override;

private:
    [[nodiscard]] bool is_templated() const { return not template_types_.empty(); }
    void write_function_definition(const Function& func) const;

    vector<string> mods_;
    Type type_;
    Token name_;
    vector<Type> template_types_;
    ParameterList params_;
    vector<StmtPtr> body_;
    ExprPtr return_expr_;

    vector<FuncPtr> funcs_;
};

#endif //FENNEC_FUNCTIONDEFINITION_H
