//
// Created by jaket on 12/11/2025.
//

#ifndef FENNEC_FUNCTION_H
#define FENNEC_FUNCTION_H

#include "utils/common.h"
#include "ParameterList.h"

class Function
{
public:
    Function(
        const Runtime& runtime,
        ModifierList mods,
        TypeInfoPtr type,
        string name,
        TypeInfoPtr template_type,
        ParameterList params,
        vector<string> output_names
    );

    Function(
        const Runtime& runtime,
        ModifierList mods,
        TypeInfoPtr type,
        string name,
        TypeInfoPtr template_type,
        ParameterList params,
        StmtPtr body,
        ExprPtr return_expr
    );

    Function(Function&& other) noexcept;

    ~Function();

    bool is_inline() const { return mods_.contains(TokenType::Inline); }
    bool is_default() const { return mods_.contains(TokenType::Default); }
    TypeInfoPtr type() const { return type_; }
    bool is_void() const;
    const string& name() const { return name_; }
    bool has_template_type() const { return template_type_ != nullptr; }
    TypeInfoPtr template_type() const { return template_type_; }
    size_t min_arity() const;
    size_t max_arity() const { return params_.size(); }
    const ParameterList& parameters() const { return params_; }
    vector<const Parameter*> in_parameters() const;
    vector<const Parameter*> out_parameters() const;
    bool has_body() const { return body_ != nullptr; }
    const StmtPtr& body() const { return body_; }
    const ExprPtr& return_expr() const { return return_expr_; }
    bool is_initialized() const { return is_initialized_; }

    void init();

    ValuePtr invoke() const;

    const vector<string>& output_names() const { return output_names_; }
    const string& output_name(const size_t i) const { return output_names_.at(i); }

    void add_nonlocal_input(const string& name, const VarPtr& var);
    void add_nonlocal_output(const string& name, const VarPtr& var);

    const unordered_map<string, VarPtr>& nonlocal_inputs() const { return nonlocal_inputs_; }
    const unordered_map<string, VarPtr>& nonlocal_outputs() const { return nonlocal_outputs_; }

    string nonlocal_name(const Parameter& param) const;

    string str() const;

private:
    ValuePtr evaluate_return() const;

    const Runtime& runtime_;
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
