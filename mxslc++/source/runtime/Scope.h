//
// Created by jaket on 28/11/2025.
//

#ifndef FENNEC_SCOPE_H
#define FENNEC_SCOPE_H

#include "utils/common.h"

class ArgumentList;

class Scope
{
public:
    Scope();
    explicit Scope(ScopePtr parent);

    ScopePtr exit()
    {
        parent_->is_youngest_ = true;
        return std::move(parent_);
    }

    mx::GraphElementPtr graph() const { return graph_; }
    pair<mx::NodeGraphPtr, FuncPtr> node_graph() const;
    void set_graph(mx::GraphElementPtr graph, FuncPtr func) { graph_ = std::move(graph); func_ = std::move(func); }
    bool is_inline() const;

    /*
     * variables
     */

    void add_variable(string name, VarPtr var);
    VarPtr get_variable(const string& name) const;
    bool has_variable(const string& name) const;
    bool is_variable_local(const VarPtr& var) const;
    bool is_variable_local(const string& name) const;
    Scope& get_defining_scope(const VarPtr& var);

    /*
     * functions
     */

    void add_function(FuncPtr func);
    vector<FuncPtr> get_functions(
        const vector<TypePtr>& return_types,
        const string& name,
        const TypePtr& template_type,
        const ArgumentList& args
        ) const;
    FuncPtr get_function(
        const vector<TypePtr>& return_types,
        const string& name,
        const TypePtr& template_type,
        const ArgumentList& args
    ) const;
    vector<FuncPtr> get_all_functions(const string& name) const;

    /*
     * types
     */

    void add_type(TypePtr type);
    void add_primitive_type(const string& name);
    void add_alias(const string& name, TypePtr type);
    bool has_type(const string& name) const;
    TypePtr resolve_type(const TypePtr& type) const;
    void resolve_fields(const TypePtr& type) const;
    TypePtr get_type(const string& name) const;

private:
    ScopePtr parent_;
    bool is_youngest_ = true;

    unordered_map<string, VarPtr> variables_;
    vector<FuncPtr> functions_;
    unordered_map<string, TypePtr> types_;

    mx::GraphElementPtr graph_;
    FuncPtr func_;
};

#endif //FENNEC_SCOPE_H
