//
// Created by jaket on 03/12/2025.
//

#include "Scope.h"

#include <cassert>

#include "ArgumentList.h"
#include "CompileError.h"
#include "Function.h"
#include "function_utils.h"
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

void Scope::set_this(const VarPtr& var)
{
    VarPtr copy = Variable::create(var);
    add_variable("this"s, std::move(copy));
}

VarPtr Scope::get_this() const
{
    return get_variable("this"s);
}

void Scope::add_function(FuncPtr func)
{
    assert(func->is_initialized());
    functions_.push_back(std::move(func));
}

namespace
{
    vector<FuncPtr> default_functions(const vector<FuncPtr>& funcs)
    {
        vector<FuncPtr> def_funcs;
        for (const FuncPtr& func : funcs)
        {
            if (func->is_default())
                def_funcs.push_back(func);
        }

        return def_funcs;
    }
}

vector<FuncPtr> Scope::get_functions(
    const vector<TypePtr>& return_types,
    const string& name,
    const TypePtr& template_type,
    const ArgumentList& args
) const
{
    vector<FuncPtr> funcs = get_matching_functions(functions_, return_types, name, template_type, args);

    if (funcs.empty() and parent_)
        funcs = parent_->get_functions(return_types, name, template_type, args);

    if (funcs.empty() and is_youngest_)
    {
        throw CompileError{missing_overload_error(get_all_functions(name), return_types, name, template_type, args)};
    }

    return funcs;
}

FuncPtr Scope::get_function(
    const vector<TypePtr>& return_types,
    const string& name,
    const TypePtr& template_type,
    const ArgumentList& args
) const
{
    const vector<FuncPtr> funcs = get_functions(return_types, name, template_type, args);
    assert(not funcs.empty());

    if (funcs.size() > 1)
    {
        const vector<FuncPtr> def_funcs = default_functions(funcs);
        if (def_funcs.size() == 1)
            return def_funcs[0];

        throw CompileError{ambiguous_overload_error(funcs)};
    }

    return funcs[0];
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
