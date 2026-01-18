//
// Created by jaket on 03/12/2025.
//

#include "Scope.h"
#include "CompileError.h"
#include "ArgumentList.h"
#include "TypeInfo.h"
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
        const vector<TypeInfoPtr>& return_types,
        const Token& name,
        const TypeInfoPtr& template_type,
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
    const vector<TypeInfoPtr>& return_types,
    const Token& name,
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
        return parent_->get_functions(return_types, name, template_type, args);

    return funcs;
}

FuncPtr Scope::get_function(
    const vector<TypeInfoPtr>& return_types,
    const Token& name,
    const TypeInfoPtr& template_type,
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

void Scope::add_type(TypeInfoPtr type)
{
    if (contains(types_, type->name()))
        throw CompileError{type->name_token(), "Type '" + type->name() + "' already defined"};
    types_.emplace(type->name(), std::move(type));
}

void Scope::add_type(const string& name)
{
    TypeInfoPtr type = std::make_shared<TypeInfo>(name);
    add_type(std::move(type));
}

bool Scope::has_type(const Token& name) const
{
    if (contains(types_, name.lexeme()))
        return true;
    if (parent_)
        return parent_->has_type(name);
    return false;
}

TypeInfoPtr Scope::get_type(const TypeInfoPtr& type) const
{
    if (type->name().empty())
    {
        vector<FieldInfo> fields;
        fields.reserve(type->field_count());
        for (const FieldInfo& field : type->fields())
            fields.emplace_back(field.modifiers(), get_type(field.type()), field.name(), field.initializer());
        return std::make_shared<TypeInfo>(type->modifiers(), type->name(), type->parent(), std::move(fields));
    }
    else
    {
        return get_type(type->name_token());
    }
}

TypeInfoPtr Scope::get_type(const Token& name) const
{
    if (contains(types_, name.lexeme()))
        return types_.at(name.lexeme());
    if (parent_)
        return parent_->get_type(name);
    throw CompileError{name, "Type '" + name.lexeme() + "' not defined"};
}

TypeInfoPtr Scope::get_type(const string& name) const
{
    return get_type(Token{name});
}

TypeInfoPtr Scope::get_type(const basic_t& value) const
{
    string type;
    if (std::holds_alternative<bool>(value))
        type = TypeInfo::Basic::Bool;
    if (std::holds_alternative<int>(value))
        type = TypeInfo::Basic::Int;
    if (std::holds_alternative<float>(value))
        type = TypeInfo::Basic::Float;
    if (std::holds_alternative<string>(value))
        type = TypeInfo::Basic::String;
    return get_type(type);
}
