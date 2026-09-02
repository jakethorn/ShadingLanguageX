//
// Created by jaket on 03/12/2025.
//

#include "runtime/Scope.h"

#include <cassert>

#include "runtime/ArgumentList.h"
#include "runtime/Function.h"
#include "runtime/FunctionQuery.h"
#include "runtime/Type.h"
#include "runtime/variables/Variable.h"
#include "utils/container_utils.h"
#include "errors/CompileError.h"
#include "runtime/interface.h"

namespace mxslc::runtime
{
    using container_utils::contains;
    using container_utils::extend;

    Scope::Scope() : Scope{string{}, nullptr} { }
    Scope::Scope(string name) : Scope{std::move(name), nullptr} { }
    Scope::Scope(ScopePtr parent) : Scope{string{}, std::move(parent)} { }
    Scope::Scope(string name, ScopePtr parent) : name_{std::move(name)}, parent_{std::move(parent)}
    {
        if (parent_)
        {
            parent_->is_youngest_ = false;
            graph_ = parent_->graph_;
            graph_func_ = parent_->graph_func_;
            func_ = parent_->func_;
        }
    }

    std::pair<mx::NodeGraphPtr, FuncPtr> Scope::node_graph() const
    {
        if (const mx::NodeGraphPtr& node_graph = std::dynamic_pointer_cast<mx::NodeGraph>(graph_))
            return {node_graph, graph_func_};

        throw CompileError{"Not in a node graph"};
    }

    bool Scope::is_inline() const
    {
        if (parent_ == nullptr)
            return true;
        return graph_func_ == parent_->graph_func_;
    }

    void Scope::add_variable(string name, VarPtr var)
    {
        if (contains(variables_, name))
            throw CompileError{"Variable already defined: " + name};

        var->set_name(name);
        var->defining_scope_ = this;
        variables_.emplace(std::move(name), std::move(var));
    }

    VarPtr Scope::get_variable(const string& name) const
    {
        if (contains(variables_, name))
            return variables_.at(name);
        if (parent_)
            return parent_->get_variable(name);
        throw CompileError{"Variable not defined: " + name};
    }

    vector<VarPtr> Scope::get_all_variables()
    {
        vector<VarPtr> vars;

        for (const auto& [name, var] : variables_)
            vars.push_back(var);

        if (parent_)
            extend(vars, parent_->get_all_variables());

        return vars;
    }

    bool Scope::has_variable(const string& name) const
    {
        return contains(variables_, name) or (parent_ and parent_->has_variable(name));
    }

    bool Scope::is_variable_local(const VarPtr& var) const
    {
        if (var->is_temporary())
            return true;
        if (var->defining_scope() == this)
            return true;
        if (parent_)
            return parent_->is_variable_local(var) and is_inline();
        throw CompileError{"Variable not defined: " + var->name()};
    }

    bool Scope::is_variable_local(const string& name) const
    {
        return is_variable_local(get_variable(name));
    }

    void Scope::add_function(FuncPtr func)
    {
        assert(func->is_initialized());
        func->defining_scope_ = this;
        functions_[func->name()].push_back(std::move(func));
    }

    FuncPtr Scope::get_function(const FunctionQuery& query) const
    {
        FuncPtr func;
        const auto it = functions_.find(*query.name);
        if (it != functions_.end())
            func = query.get_match(it->second, /*throw_on_fail*/false);

        if (func)
            return func;
        if (parent_)
            return parent_->get_function(query);
        return nullptr;
    }

    vector<FuncPtr> Scope::get_functions(const FunctionQuery& query) const
    {
        vector<FuncPtr> funcs;
        const auto it = functions_.find(*query.name);
        if (it != functions_.end())
            funcs = query.get_matches(it->second);

        if (not funcs.empty())
            return funcs;
        if (parent_)
            return parent_->get_functions(query);
        return funcs;
    }

    bool Scope::has_function(const FuncPtr& func) const
    {
        const auto it = functions_.find(func->name());
        if (it != functions_.end() and contains(it->second, func))
            return true;
        return parent_ and parent_->has_function(func);
    }

    bool Scope::has_function(const FunctionQuery& query) const
    {
        return not get_functions(query).empty();
    }

    void Scope::add_type(TypePtr type)
    {
        assert(type->has_name());

        if (contains(types_, type->name()))
            throw CompileError{"Type '" + type->name() + "' already defined"};

        resolve_fields(type);
        type->set_resolved();

        types_.emplace(type->name(), std::move(type));
    }

    void Scope::add_primitive_type(const string& name)
    {
        TypePtr type = create_type(name);
        add_type(std::move(type));
    }

    void Scope::add_alias(const string& name, TypePtr type)
    {
        if (contains(types_, name))
            throw CompileError{"Type '" + name + "' already defined"};

        type = resolve_type(type);
        types_.emplace(name, type);
    }

    bool Scope::has_type(const string& name) const
    {
        if (contains(types_, name))
            return true;
        if (parent_)
            return parent_->has_type(name);
        return false;
    }

    TypePtr Scope::resolve_type(const TypePtr& type) const
    {
        if (type->is_void())
            return Type::Void;

        if (type->is_auto())
            return Type::Auto;

        if (type->has_name())
        {
            return get_type(type->name());
        }
        else
        {
            resolve_fields(type);
            type->set_resolved();
            return type;
        }
    }

    TypePtr Scope::get_type(const string& name) const
    {
        if (contains(types_, name))
            return types_.at(name);
        if (parent_)
            return parent_->get_type(name);
        throw CompileError{"Type '" + name + "' not defined"};
    }

    void Scope::resolve_fields(const TypePtr& type) const
    {
        for (Field& field : type->fields_)
            field.type_ = resolve_type(field.type_);
    }
}
