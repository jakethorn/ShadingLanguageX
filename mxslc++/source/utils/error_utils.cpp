//
// Created by jaket on 10/04/2026.
//

#include "error_utils.h"

#include <cassert>

#include "statements/Statement.h"
#include "runtime/ArgumentList.h"
#include "runtime/Function.h"
#include "runtime/Function.h"
#include "runtime/Type.h"

string missing_overload_error(
    const FuncPtr& func,
    const vector<TypePtr>& return_types,
    const string& name,
    const TypePtr& template_type,
    const ArgumentList& args
)
{
    string result = func->str() + "\n";

    if (not return_types.empty() and not contains(return_types, func->return_type()))
        result += "    Return type does not match\n";

    if (name != func->name())
        result += "    Name does not match\n";

    if (template_type)
    {
        if (not func->has_template_type() or not template_type->is_compatible(func->template_type()))
            result += "    Template type does not match\n";
    }

    if (args.size() > func->max_arity())
        result += "    Too many arguments\n";

    if (args.size() < func->min_arity())
        result += "    Too few arguments\n";

    for (const Argument& arg : args)
    {
        try
        {
            const Parameter& param = func->parameters()[arg];
            if (not arg.is_initialized())
                result += "    Parameter '" + param.name() + "' was given an argument of the wrong type\n";
        }
        catch (const std::out_of_range&)
        {

        }
    }

    return result;
}

string missing_overload_error(
    const vector<FuncPtr>& funcs,
    const vector<TypePtr>& return_types,
    const string& name,
    const TypePtr& template_type,
    const ArgumentList& args
)
{
    if (funcs.empty())
    {
        return "No functions found with the name '" + name + "'";
    }

    string result = "No matching overload for function '" + name + "'\n";
    result += "Potential matches:\n";
    for (const FuncPtr& func : funcs)
        result += missing_overload_error(func, return_types, name, template_type, args) + "\n";
    return result;
}

string ambiguous_overload_error(const vector<FuncPtr>& funcs)
{
    assert(not funcs.empty());

    string result = "More than one matching overload for function '" + funcs[0]->name() + "'\n";
    result += "Potential matches:\n";
    for (const FuncPtr& func : funcs)
        result += func->str() + "\n";
    return result;
}
