//
// Created by jaket on 03/01/2026.
//

#ifndef FENNEC_FUNCTIONDEFINITION_H
#define FENNEC_FUNCTIONDEFINITION_H

//#include "utils/common.h"
//#include "Statement.h"
//#include "Token.h"
//#include "runtime/ParameterList.h"
//
//class FunctionDefinition final : public Statement
//{
//public:
//    FunctionDefinition(const Runtime& runtime, ModifierList mods, TypeInfoPtr type, Token name, vector<TypeInfoPtr> template_types, ParameterList params, StmtPtr body, ExprPtr return_expr);
//
//    StmtPtr instantiate_template_types(const TypeInfoPtr& template_type) const override;
//
//protected:
//    void execute_impl() const override;
//
//private:
//    const string& name() const { return token_.lexeme(); }
//    bool is_templated() const { return not template_types_.empty(); }
//    void write_function_definition(const FuncPtr& func) const;
//    void create_out_variables(const FuncPtr& func) const;
//    void add_parameters_to_scope(const FuncPtr& func) const;
//
//    ModifierList mods_;
//    TypeInfoPtr type_;
//    vector<TypeInfoPtr> template_types_;
//    ParameterList params_;
//    StmtPtr body_;
//    ExprPtr return_expr_;
//
//    vector<FuncPtr> funcs_;
//};

#endif //FENNEC_FUNCTIONDEFINITION_H
