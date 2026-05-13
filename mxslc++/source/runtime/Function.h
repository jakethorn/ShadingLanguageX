//
// Created by jaket on 16/04/2026.
//

#ifndef MXSLC_FUNCTION_H
#define MXSLC_FUNCTION_H

#include "ModifierList.h"
#include "ParameterList.h"

class Function
{
public:
    Function(
        ModifierList mods,
        TypePtr return_type,
        string name,
        TypePtr template_type,
        ParameterList params,
        mx::NodeDefPtr node_def
    );

    Function(
        ModifierList mods,
        TypePtr return_type,
        string name,
        TypePtr template_type,
        ParameterList params,
        StmtPtr body,
        ExprPtr return_expr
    );

    Function(Function&& other) noexcept;

    ~Function();

    bool is_inline() const { return mods_.contains(TokenType::Inline); }
    bool is_default() const { return mods_.contains(TokenType::Default); }
    TypePtr return_type() const { return return_type_; }
    bool is_void() const;
    const string& name() const { return name_; }
    bool has_template_type() const { return template_type_ != nullptr; }
    TypePtr template_type() const { return template_type_; }
    size_t min_arity() const;
    size_t max_arity() const { return params_.size(); }
    const ParameterList& parameters() const { return params_; }
    bool is_defined() const { return body_ != nullptr; }
    mx::NodeDefPtr node_def() const { return node_def_; }
    bool is_initialized() const { return is_initialized_; }

    void set_node_def(mx::NodeDefPtr node_def);
    vector<string> output_names() const;

    void init();

    VarPtr invoke() const;

    void add_nonlocal_input(VarPtr var) { nonlocal_inputs_.push_back(std::move(var)); }
    void add_nonlocal_output(VarPtr var) { nonlocal_outputs_.push_back(std::move(var)); }
    const vector<VarPtr>& nonlocal_inputs() const { return nonlocal_inputs_; }
    const vector<VarPtr>& nonlocal_outputs() const { return nonlocal_outputs_; }

    TypePtr class_type() const { return class_type_.lock(); }
    void set_class_type(weak_ptr<Type> type) { class_type_ = std::move(type); }
    bool has_class_type() const { return not class_type_.expired(); }

    string str() const;

private:
    ModifierList mods_;
    TypePtr return_type_;
    string name_;
    TypePtr template_type_;
    ParameterList params_;
    StmtPtr body_;
    ExprPtr return_expr_;
    mx::NodeDefPtr node_def_;
    bool is_initialized_ = false;

    vector<VarPtr> nonlocal_inputs_;
    vector<VarPtr> nonlocal_outputs_;

    weak_ptr<Type> class_type_;
};

#endif //MXSLC_FUNCTION_H
