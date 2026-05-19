//
// Created by jaket on 06/05/2026.
//

#include "FunctionQuery.h"

#include "CompileError.h"
#include "Function.h"
#include "Type.h"

FunctionQuery::FunctionQuery(const string& name) : name{&name}
{
    
}

FunctionQuery::FunctionQuery(const string& name, const TypePtr& template_type)
    : name{&name}, template_type{&template_type}
{

}

FunctionQuery::FunctionQuery(const vector<TypePtr>& return_types, const string& name, const TypePtr& template_type, const ArgumentList& args)
    : return_types{&return_types}, name{&name}, template_type{&template_type}, args{&args}
{

}

FunctionQuery::FunctionQuery(const TypePtr& class_type, const vector<TypePtr>& return_types, const string& name, const TypePtr& template_type, const ArgumentList& args)
    : class_type{&class_type}, return_types{&return_types}, name{&name}, template_type{&template_type}, args{&args}
{
    
}

bool FunctionQuery::is_match(const FuncPtr& func) const
{
    return class_type_matches(func) and
            return_type_matches(func) and
            name_matches(func) and
            template_type_matches(func) and
            arguments_match(func);
}

bool FunctionQuery::has_match(const vector<FuncPtr>& funcs) const
{
    for (const FuncPtr& func : funcs)
    {
        if (is_match(func))
            return true;
    }

    return false;
}

FuncPtr FunctionQuery::get_match(const vector<FuncPtr>& funcs, const bool throw_on_fail) const
{
    vector<FuncPtr> matches = get_matches(funcs);
    if (matches.empty())
    {
        if (throw_on_fail)
            throw CompileError{"No matches found"s};
        else
            return nullptr;
    }
    else if (matches.size() == 1)
    {
        return matches[0];
    }
    else
    {
        vector<FuncPtr> default_funcs = get_default_functions(matches);
        if (default_funcs.size() == 1)
        {
            return default_funcs[0];
        }
        else
        {
            if (throw_on_fail)
                throw CompileError{"Too many matches found"s};
            else
                return nullptr;
        }
    }
}

vector<FuncPtr> FunctionQuery::get_matches(const vector<FuncPtr>& funcs) const
{
    vector<FuncPtr> matches;
    for (const FuncPtr& func : funcs)
    {
        if (is_match(func))
            matches.push_back(func);
    }
    return matches;
}

bool FunctionQuery::class_type_matches(const FuncPtr& func) const
{
    if (class_type == nullptr)
        return true;
    return func->class_type() == *class_type;
}

bool FunctionQuery::return_type_matches(const FuncPtr& func) const
{
    // empty means any type
    if (return_types == nullptr or return_types->empty())
        return true;
    return func->return_type()->is_compatible(*return_types);
}

bool FunctionQuery::name_matches(const FuncPtr& func) const
{
    if (name == nullptr)
        return true;
    return func->name() == *name;
}

bool FunctionQuery::template_type_matches(const FuncPtr& func) const
{
    // nullptr means any type
    if (template_type == nullptr or *template_type == nullptr)
        return true;
    return func->template_type() == *template_type;
}

bool FunctionQuery::arguments_match(const FuncPtr& func) const
{
    if (args == nullptr)
        return true;

    if (args->size() > func->max_arity() or args->size() < func->min_arity())
        return false;

    for (const Argument& arg : *args)
    {
        TypePtr param_type = func->parameters()[arg].type();
        if (arg.is_initialized() and not param_type->is_compatible(arg.type()))
            return false;
    }

    return true;
}

vector<FuncPtr> FunctionQuery::get_default_functions(const vector<FuncPtr>& funcs) const
{
    vector<FuncPtr> default_funcs;
    for (const FuncPtr& func : funcs)
    {
        if (func->is_default())
            default_funcs.push_back(func);
    }

    return default_funcs;
}
