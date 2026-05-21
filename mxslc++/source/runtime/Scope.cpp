//
// Created by jaket on 03/12/2025.
//

#include "Scope.h"

#include <cassert>

#include "ArgumentList.h"
#include "CompileError.h"
#include "Function.h"
#include "FunctionQuery.h"
#include "Type.h"
#include "Variable.h"
#include "utils/error_utils.h"
#include "utils/template_utils.h"

Scope::Scope() = default;

Scope::Scope(ScopePtr parent) : Scope{""s, std::move(parent)}
{

}

Scope::Scope(string name, ScopePtr parent) : parent_{std::move(parent)}
{
    name_ = std::move(name);
    parent_->is_youngest_ = false;
    graph_ = parent_->graph_;
    func_ = parent_->func_;
}

pair<mx::NodeGraphPtr, FuncPtr> Scope::node_graph() const
{
    if (const mx::NodeGraphPtr& node_graph = std::dynamic_pointer_cast<mx::NodeGraph>(graph_))
        return {node_graph, func_};

    throw CompileError{"Not in a node graph"s};
}

bool Scope::is_inline() const
{
    if (parent_ == nullptr)
        return true;
    return func_ == parent_->func_;
}

void Scope::add_variable(string name, VarPtr var)
{
    if (contains(variables_, name))
        throw CompileError{"Variable already defined: " + name};

    var->set_name(name);
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

bool Scope::has_variable(const string& name) const
{
    return contains(variables_, name) or (parent_ and parent_->has_variable(name));
}

bool Scope::is_variable_local(const VarPtr& var) const
{
    return is_variable_local(var->oldest()->name());
}

bool Scope::is_variable_local(const string& name) const
{
    if (contains(variables_, name))
        return true;
    if (parent_)
        return parent_->is_variable_local(name) and is_inline();
    throw CompileError{"Variable not defined: " + name};
}

Scope& Scope::get_defining_scope(const VarPtr& var)
{
    if (contains(variables_, var->name()))
        return *this;
    if (parent_)
        return parent_->get_defining_scope(var);
    throw CompileError{"Variable not defined: " + var->name()};
}

void Scope::add_function(FuncPtr func)
{
    assert(func->is_initialized());
    functions_.push_back(std::move(func));
}

FuncPtr Scope::get_function(const FunctionQuery& query, const bool throw_on_fail) const
{
    FuncPtr func = query.get_match(functions_, false);
    if (func)
        return func;
    if (parent_)
        return parent_->get_function(query, throw_on_fail);
    if (throw_on_fail)
        throw CompileError{"Function not defined"s};
    else
        return nullptr;
}

vector<FuncPtr> Scope::get_functions(const FunctionQuery& query, const bool throw_on_fail) const
{
    vector<FuncPtr> funcs = query.get_matches(functions_);
    if (not funcs.empty())
        return funcs;
    if (parent_)
        return parent_->get_functions(query, throw_on_fail);
    if (throw_on_fail)
        throw CompileError{"Functions not defined"s};
    else
        return vector<FuncPtr>{};
}

Scope& Scope::get_defining_scope(const FuncPtr& func)
{
    if (contains(functions_, func))
        return *this;
    if (parent_)
        return parent_->get_defining_scope(func);
    throw CompileError{"Function not defined: " + func->name()};
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
    TypePtr type = std::make_shared<Type>(name);
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

void Scope::resolve_fields(const TypePtr& type) const
{
    for (Field& field : type->fields_)
        field.type_ = resolve_type(field.type_);
}

TypePtr Scope::get_type(const string& name) const
{
    if (contains(types_, name))
        return types_.at(name);
    if (parent_)
        return parent_->get_type(name);
    throw CompileError{"Type '" + name + "' not defined"};
}

vector<FuncPtr> Scope::get_all_functions(const string& name) const
{
    vector<FuncPtr> funcs;
    for (const FuncPtr& func : functions_)
    {
        if (name == func->name())
            funcs.push_back(func);
    }

    if (parent_)
    {
        vector<FuncPtr> parent_funcs = parent_->get_all_functions(name);
        funcs.insert(funcs.cend(), parent_funcs.begin(), parent_funcs.end());
    }

    return funcs;
}
