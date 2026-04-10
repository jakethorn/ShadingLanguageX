//
// Created by jaket on 12/11/2025.
//

#ifndef FENNEC_FUNCTION_H
#define FENNEC_FUNCTION_H

#include "utils/common.h"
#include "utils/template_utils.h"
#include "Token.h"
#include "ParameterList.h"

class Function
{
public:
    Function(
        ModifierList mods,
        TypeInfoPtr type,
        Token name,
        TypeInfoPtr template_type,
        ParameterList params,
        vector<string> output_names
    );

    Function(
        ModifierList mods,
        TypeInfoPtr type,
        Token name,
        TypeInfoPtr template_type,
        ParameterList params,
        StmtPtr body,
        ExprPtr return_expr
    );

    Function(Function&& other) noexcept;
    Function& operator=(Function&& other) noexcept;

    Function(const Function&) = delete;
    Function& operator=(const Function&) = delete;

    ~Function();

    [[nodiscard]] bool is_inline() const { return mods_.contains("inline"s); }
    [[nodiscard]] bool is_default() const { return mods_.contains("default"s); }
    [[nodiscard]] TypeInfoPtr type() const { return type_; }
    [[nodiscard]] const string& name() const { return name_.lexeme(); }
    [[nodiscard]] bool has_template_type() const { return template_type_ != nullptr; }
    [[nodiscard]] TypeInfoPtr template_type() const { return template_type_; }
    [[nodiscard]] size_t min_arity() const;
    [[nodiscard]] size_t max_arity() const { return params_.size(); }
    [[nodiscard]] const ParameterList& parameters() const { return params_; }
    [[nodiscard]] bool has_body() const { return body_ != nullptr; }
    [[nodiscard]] const StmtPtr& body() const { return body_; }
    [[nodiscard]] const ExprPtr& return_expr() const { return return_expr_; }
    [[nodiscard]] const Token& name_token() const { return name_; }
    [[nodiscard]] bool is_initialized() const { return is_initialized_; }

    [[nodiscard]] const vector<string>& output_names() const { return output_names_; }
    [[nodiscard]] const string& output_name(const size_t i) const { return output_names_.at(i); }

    void add_nonlocal_input(const string& name, const VarPtr& var);
    void add_nonlocal_output(const string& name, const VarPtr& var);

    const unordered_map<string, VarPtr>& nonlocal_inputs() const { return nonlocal_inputs_; }
    const unordered_map<string, VarPtr>& nonlocal_outputs() const { return nonlocal_outputs_; }

    void init(const Runtime& runtime);

    string str() const;

private:
    ModifierList mods_;
    TypeInfoPtr type_;
    Token name_;
    TypeInfoPtr template_type_;
    ParameterList params_;
    StmtPtr body_;
    ExprPtr return_expr_;

    bool is_initialized_ = false;

    vector<string> output_names_;
    unordered_map<string, VarPtr> nonlocal_inputs_;
    unordered_map<string, VarPtr> nonlocal_outputs_;
};

#endif //FENNEC_FUNCTION_H
