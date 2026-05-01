//
// Created by jaket on 28/11/2025.
//

#ifndef FENNEC_SCOPE_H
#define FENNEC_SCOPE_H

#include "utils/common.h"
#include "Function.h"

class ArgumentList;

class Scope
{
public:
    Scope();
    explicit Scope(ScopePtr parent);
    Scope(ScopePtr parent, bool is_inline);

    ScopePtr exit()
    {
        return std::move(parent_);
    }

    bool is_inline() const { return is_inline_; }

    /*
     * variables
     */

    void add_variable(string name, VarPtr var);
    VarPtr get_variable(const string& name) const;
    bool is_variable_local(const VarPtr& var) const;
    bool is_variable_local(const string& name) const;

    /*
     * functions
     */

    void add_function(FuncPtr func);
    vector<FuncPtr> get_functions(
        const vector<TypeInfoPtr>& return_types,
        const string& name,
        const TypeInfoPtr& template_type,
        const ArgumentList& args
        ) const;
    FuncPtr get_function(
        const vector<TypeInfoPtr>& return_types,
        const string& name,
        const TypeInfoPtr& template_type,
        const ArgumentList& args
    ) const;
    vector<FuncPtr> get_all_functions(const string& name) const;

    /*
     * types
     */

    void add_type(TypeInfoPtr type);
    void add_basic_type(const string& name);
    void add_alias(const string& name, TypeInfoPtr type);
    bool has_type(const string& name) const;
    TypeInfoPtr resolve_type(const TypeInfoPtr& type) const;
    void resolve_fields(const TypeInfoPtr& type) const;
    TypeInfoPtr get_type(const string& name) const;

private:
    ScopePtr parent_;
    bool is_inline_;

    unordered_map<string, VarPtr> variables_;
    vector<FuncPtr> functions_;
    unordered_map<string, TypeInfoPtr> types_;
};

#endif //FENNEC_SCOPE_H
