//
// Created by jaket on 03/12/2025.
//

#include <cassert>

#include "Scope.h"
#include "statements/Statement.h"
#include "CompileError.h"
#include "ArgumentList.h"
#include "Function.h"
#include "TypeInfo.h"
#include "Variable.h"
#include "utils/error_utils.h"
#include "utils/template_utils.h"

Scope::Scope() : Scope{nullptr} { }
Scope::Scope(ScopePtr parent) : Scope{std::move(parent), false} { }
Scope::Scope(ScopePtr parent, const bool is_inline) : parent_{std::move(parent)}, is_inline_{is_inline}
{

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

bool Scope::is_variable_local(const VarPtr& var) const
{
    VarPtr tmp = var;
    while (tmp->parent())
        tmp = tmp->parent();
    return is_variable_local(tmp->name());
}

bool Scope::is_variable_local(const string& name) const
{
    if (contains(variables_, name))
    {
        return true;
    }

    if (parent_)
    {
        return parent_->is_variable_local(name) and is_inline_;
    }

    throw CompileError{"Variable not defined: " + name};
}

void Scope::add_function(FuncPtr func)
{
    assert(func->is_initialized());
    functions_.push_back(std::move(func));
}

namespace
{
    bool is_match(
        const Function& func,
        const vector<TypeInfoPtr>& return_types,
        const string& name,
        const TypeInfoPtr& template_type,
        const ArgumentList& args
    )
    {
        if (name != func.name())
            return false;

        if (not return_types.empty() and not func.return_type()->is_compatible(return_types))
            return false;

        if (template_type)
        {
            if (not func.has_template_type())
                return false;
            if (not template_type->is_compatible(func.template_type()))
                return false;
        }

        if (args.size() > func.max_arity() or args.size() < func.min_arity())
            return false;

        for (const Argument& arg : args)
        {
            TypeInfoPtr param_type = func.parameters()[arg].type();
            if (arg.is_initialized() and not param_type->is_compatible(arg.type()))
                return false;
        }

        return true;
    }

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
    const vector<TypeInfoPtr>& return_types,
    const string& name,
    const TypeInfoPtr& template_type,
    const ArgumentList& args
) const
{
    vector<FuncPtr> funcs;
    for (const FuncPtr& func : functions_)
    {
        if (is_match(*func, return_types, name, template_type, args))
            funcs.push_back(func);
    }

    if (funcs.empty() and parent_)
        funcs = parent_->get_functions(return_types, name, template_type, args);

    if (funcs.empty())
    {
        throw CompileError{missing_overload_error(get_all_functions(name), return_types, name, template_type, args)};
    }

    return funcs;
}

FuncPtr Scope::get_function(
    const vector<TypeInfoPtr>& return_types,
    const string& name,
    const TypeInfoPtr& template_type,
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

void Scope::add_type(TypeInfoPtr type)
{
    assert(type->has_name());

    if (contains(types_, type->name()))
        throw CompileError{"Type '" + type->name() + "' already defined"};

    resolve_fields(type);
    type->set_resolved();

    types_.emplace(type->name(), std::move(type));
}

void Scope::add_basic_type(const string& name)
{
    TypeInfoPtr type = std::make_shared<TypeInfo>(name);
    add_type(std::move(type));
}

void Scope::add_alias(const string& name, TypeInfoPtr type)
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

TypeInfoPtr Scope::resolve_type(const TypeInfoPtr& type) const
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

void Scope::resolve_fields(const TypeInfoPtr& type) const
{
    for (FieldInfo& field : type->fields_)
        field.type_ = resolve_type(field.type_);
}

TypeInfoPtr Scope::get_type(const string& name) const
{
    if (contains(types_, name))
        return types_.at(name);
    if (parent_)
        return parent_->get_type(name);
    throw CompileError{"Type '" + name + "' not defined"};
}
