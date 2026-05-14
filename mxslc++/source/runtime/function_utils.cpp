//
// Created by jaket on 06/05/2026.
//

#include "function_utils.h"

#include "CompileError.h"
#include "Function.h"
#include "Type.h"
#include "utils/error_utils.h"

bool is_matching_function(const FuncPtr& func, const string& name)
{
    return name == func->name();
}

bool is_matching_function(const FuncPtr& func, const vector<TypePtr>& return_types, const string& name, const TypePtr& template_type, const ArgumentList& args)
{
    if (name != func->name())
        return false;

    if (not return_types.empty())
    {
        if (not func->return_type()->is_compatible(return_types))
            return false;
    }

    if (template_type)
    {
        if (not func->has_template_type())
            return false;
        if (not template_type->is_compatible(func->template_type()))
            return false;
    }

    if (args.size() > func->max_arity() or args.size() < func->min_arity())
        return false;

    for (const Argument& arg : args)
    {
        TypePtr param_type = func->parameters()[arg].type();
        if (arg.is_initialized() and not param_type->is_compatible(arg.type()))
            return false;
    }

    return true;
}

vector<FuncPtr> get_matching_functions(const vector<FuncPtr>& funcs, const string& name)
{
    vector<FuncPtr> matching;
    for (const FuncPtr& func : funcs)
    {
        if (is_matching_function(func, name))
            matching.push_back(func);
    }

    return matching;
}

vector<FuncPtr> get_matching_functions(const vector<FuncPtr>& funcs, const vector<TypePtr>& return_types, const string& name, const TypePtr& template_type, const ArgumentList& args)
{
    vector<FuncPtr> matching;
    for (const FuncPtr& func : funcs)
    {
        if (is_matching_function(func, return_types, name, template_type, args))
            matching.push_back(func);
    }

    return matching;
}

FuncPtr get_matching_function(const vector<FuncPtr>& funcs, const vector<TypePtr>& return_types, const string& name, const TypePtr& template_type, const ArgumentList& args)
{
    vector<FuncPtr> matching = get_matching_functions(funcs, return_types, name, template_type, args);

    if (matching.empty())
        throw CompileError{missing_overload_error(get_matching_functions(funcs, name), return_types, name, template_type, args)};

    if (matching.size() > 1)
        throw CompileError{ambiguous_overload_error(matching)};

    return matching[0];
}
