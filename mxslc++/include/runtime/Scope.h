//
// Created by jaket on 28/11/2025.
//

#ifndef FENNEC_SCOPE_H
#define FENNEC_SCOPE_H

#include <MaterialXCore/Node.h>

#include "common.h"

namespace mxslc::runtime
{
    class FunctionQuery;

    class Scope
    {
    public:
        Scope();
        explicit Scope(string name);
        explicit Scope(ScopePtr parent);
        Scope(string name, ScopePtr parent);

        ScopePtr exit()
        {
            parent_->is_youngest_ = true;
            return std::move(parent_);
        }

        const string& name() const { return name_; }

        bool has_parent() const { return parent_ != nullptr; }
        Scope* parent() const { return parent_.get(); }
        size_t depth() const;

        mx::GraphElementPtr graph() const { return graph_; }
        pair<mx::NodeGraphPtr, FuncPtr> node_graph() const;
        void set_graph(mx::GraphElementPtr graph, FuncPtr func) { graph_ = std::move(graph); graph_func_ = func; func_ = func; }

        FuncPtr function() const { return func_; }
        void set_function(FuncPtr func) { func_ = std::move(func); }

        // true for global scope, inline functions, loops and if statements
        bool is_inline() const;

        // true if current scope is or is inside of an inline function
        bool is_inside_inline_function() const;

        void poison(string name);
        void throw_if_poisoned(const string& name) const;

        /*
         * variables
         */

        void add_variable(string name, VarPtr var);
        VarPtr get_variable(const string& name) const;
        vector<VarPtr> get_all_variables();
        bool has_variable(const string& name) const;
        bool defines_variable(const string& name) const;
        bool is_variable_local(const VarPtr& var) const;
        bool is_variable_local(const string& name) const;

        /*
         * functions
         */

        void add_function(FuncPtr func);
        void remove_function(const FuncPtr& func);
        FuncPtr get_function(const FunctionQuery& query) const;
        vector<FuncPtr> get_functions(const FunctionQuery& query) const;
        bool has_function(const FuncPtr& func) const;
        bool has_function(const FunctionQuery& query) const;

        /*
         * types
         */

        void add_type(TypePtr type);
        void add_alias(const string& name, TypePtr type);
        bool has_type(const string& name) const;
        TypePtr resolve_type(const TypePtr& type) const;
        TypePtr get_type(const string& name) const;

        /*
         * reflection api
         */

        vector<string> get_all_function_names() const;
        vector<string> get_all_type_names() const;

    private:
        VarPtr find_variable(const string& name) const;
        TypePtr find_type(const string& name) const;
        bool is_poisoned(const string& name) const;
        void resolve_fields(const TypePtr& type) const;

        string name_;
        ScopePtr parent_;
        bool is_youngest_{true};

        unordered_map<string, VarPtr> variables_;
        unordered_map<string, vector<FuncPtr>> functions_;
        unordered_map<string, TypePtr> types_;
        unordered_set<string> poisoned_names_;

        mx::GraphElementPtr graph_;
        FuncPtr graph_func_;
        FuncPtr func_;
    };
}

#endif //FENNEC_SCOPE_H
