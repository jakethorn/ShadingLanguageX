//
// Created by jaket on 03/12/2025.
//

#include "Scope.h"
#include "CompileError.h"
#include "ArgumentList.h"
#include "utils/template_utils.h"

Scope::Scope() : parent_{nullptr} { }
Scope::Scope(ScopePtr parent) : parent_{std::move(parent)} { }

void Scope::add_variable(Variable&& var)
{
    if (contains(variables_, var.name()))
        throw CompileError{var.token(), "Variable already defined: " + var.name()};

    variables_.emplace(var.name(), std::move(var));
}

void Scope::set_variable(Variable&& var)
{
    if (contains(variables_, var.name()))
    {
        variables_.insert_or_assign(var.name(), std::move(var));
        return;
    }

    if (parent_)
    {
        parent_->set_variable(std::move(var));
        return;
    }

    throw CompileError{var.token(), "Variable not defined: " + var.name()};
}

const Variable& Scope::get_variable(const Token& name) const
{
    if (contains(variables_, name.lexeme()))
    {
        return variables_.at(name.lexeme());
    }

    if (parent_)
    {
        return parent_->get_variable(name);
    }

    throw CompileError{name, "Variable not defined: " + name.lexeme()};
}

void Scope::add_function(FuncPtr func)
{
    functions_.push_back(std::move(func));
}

namespace
{
    bool is_match(
        const Function& func,
        const vector<Type>& return_types,
        const Token& name,
        const optional<Type>& template_type,
        const ArgumentList& args
    )
    {
        if (not return_types.empty() and not contains(return_types, func.type()))
            return false;

        if (name.lexeme() != func.name())
            return false;

        if (template_type)
        {
            if (not func.has_template_type())
                return false;
            if (*template_type != func.template_type())
                return false;
        }

        if (args.size() > func.arity())
            return false;

        for (const Argument& arg : args)
        {
            if (arg.is_initialized() and func.parameters()[arg].type() != arg.type())
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
    const vector<Type>& return_types,
    const Token& name,
    const optional<Type>& template_type,
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
        return parent_->get_functions(return_types, name, template_type, args);

    return funcs;
}

FuncPtr Scope::get_function(
    const vector<Type>& return_types,
    const Token& name,
    const optional<Type>& template_type,
    const ArgumentList& args
) const
{
    const vector<FuncPtr> funcs = get_functions(return_types, name, template_type, args);

    if (funcs.empty())
        throw CompileError{name, "Function not defined: " + name.lexeme()};

    if (funcs.size() > 1)
    {
        vector<FuncPtr> def_funcs = default_functions(funcs);
        if (def_funcs.size() == 1)
            return def_funcs[0];

        throw CompileError{name, "Ambiguous function: " + name.lexeme()};
    }

    return funcs[0];
}

void Scope::add_type(Type&& type)
{
    if (contains(types_, type.str()))
        throw CompileError{"Type already defined: " + type.str()};

    types_.emplace(type.str(), std::move(type));
}

bool Scope::has_type(const Type& type) const
{
    if (type.is_complex())
    {
        bool has = true;
        for (const Type& t : type.subtypes())
            has &= has_type(t);
        return has;
    }
    else
    {
        return has_type(type.str());
    }
}

bool Scope::has_type(const string& name) const
{
    if (contains(types_, name))
    {
        return true;
    }

    if (parent_)
    {
        return parent_->has_type(name);
    }

    return false;
}
