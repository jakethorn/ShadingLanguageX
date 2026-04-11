//
// Created by jaket on 12/11/2025.
//

#ifndef FENNEC_FUNCTION_H
#define FENNEC_FUNCTION_H

#include "utils/common.h"
#include "utils/template_utils.h"
#include "ParameterList.h"

class Function
{
public:
    Function(
        ModifierList mods,
        TypeInfoPtr type,
        string name,
        TypeInfoPtr template_type,
        ParameterList params,
        vector<string> output_names
    );

    Function(
        ModifierList mods,
        TypeInfoPtr type,
        string name,
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

    bool is_inline() const { return mods_.contains(TokenType::Inline); }
    bool is_default() const { return mods_.contains(TokenType::Default); }
    TypeInfoPtr type() const { return type_; }
    const string& name() const { return name_; }
    bool has_template_type() const { return template_type_ != nullptr; }
    TypeInfoPtr template_type() const { return template_type_; }
    size_t min_arity() const;
    size_t max_arity() const { return params_.size(); }
    const ParameterList& parameters() const { return params_; }
    bool has_body() const { return body_ != nullptr; }
    const StmtPtr& body() const { return body_; }
    const ExprPtr& return_expr() const { return return_expr_; }
    bool is_initialized() const { return is_initialized_; }

    const vector<string>& output_names() const { return output_names_; }
    const string& output_name(const size_t i) const { return output_names_.at(i); }

    void add_nonlocal_input(const string& name, const VarPtr& var);
    void add_nonlocal_output(const string& name, const VarPtr& var);

    const unordered_map<string, VarPtr>& nonlocal_inputs() const { return nonlocal_inputs_; }
    const unordered_map<string, VarPtr>& nonlocal_outputs() const { return nonlocal_outputs_; }

    void init(const Runtime& runtime);

    string str() const;

private:
    ModifierList mods_;
    TypeInfoPtr type_;
    string name_;
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
