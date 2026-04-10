//
// Created by jaket on 28/11/2025.
//

#ifndef FENNEC_SCOPE_H
#define FENNEC_SCOPE_H

#include "utils/common.h"
#include "Function.h"

class Token;
class ArgumentList;

class Scope
{
public:
    Scope();
    explicit Scope(ScopePtr parent);
    Scope(ScopePtr parent, bool is_inline);

    ScopePtr exit()
    {
        parent_->set_current(true);
        return std::move(parent_);
    }

    bool is_current() const { return is_current_; }
    bool is_inline() const { return is_inline_; }

    void set_current(const bool is_current) { is_current_ = is_current; }

    void add_variable(VarPtr var);
    void set_variable(VarPtr var);
    [[nodiscard]] VarPtr get_variable(const Token& name) const;
    [[nodiscard]] bool is_variable_inline(const VarPtr& var) const;
    [[nodiscard]] bool is_variable_inline(const Token& name) const;

    void add_function(FuncPtr func);
    [[nodiscard]] vector<FuncPtr> get_functions(
        const vector<TypeInfoPtr>& return_types,
        const Token& name,
        const TypeInfoPtr& template_type,
        const ArgumentList& args
        ) const;
    [[nodiscard]] FuncPtr get_function(
        const vector<TypeInfoPtr>& return_types,
        const Token& name,
        const TypeInfoPtr& template_type,
        const ArgumentList& args
    ) const;

    [[nodiscard]] vector<FuncPtr> get_all_functions(const Token& name) const;

    void add_type(TypeInfoPtr type);
    void add_basic_type(const string& name);
    void add_alias(const Token& name, TypeInfoPtr type);
    [[nodiscard]] bool has_type(const Token& name) const;
    [[nodiscard]] bool has_type(const string& name) const;
    [[nodiscard]] TypeInfoPtr resolve_type(const TypeInfoPtr& type) const;
    void resolve_fields(const TypeInfoPtr& type) const;
    [[nodiscard]] TypeInfoPtr get_type(const Token& name) const;
    [[nodiscard]] TypeInfoPtr get_type(const string& name) const;

private:
    ScopePtr parent_;
    bool is_current_;
    bool is_inline_;

    unordered_map<string, VarPtr> variables_;
    vector<FuncPtr> functions_;
    unordered_map<string, TypeInfoPtr> types_;
};

#endif //FENNEC_SCOPE_H
