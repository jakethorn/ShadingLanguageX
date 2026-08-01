//
// Created by jaket on 06/05/2026.
//

#include "runtime/FunctionQuery.h"

#include "runtime/Function.h"
#include "runtime/Type.h"
#include "runtime/ArgumentList.h"
#include "errors/CompileError.h"

namespace mxslc::runtime
{
    FunctionQuery::FunctionQuery(const string& name) : name{&name}
    {

    }

    FunctionQuery::FunctionQuery(const string& name, const TypePtr& template_type)
        : name{&name}, template_type{&template_type}
    {

    }

    FunctionQuery::FunctionQuery(const vector<TypePtr>& return_types, const string& name, const bool& is_parameterless)
        : return_types{&return_types}, name{&name}, is_parameterless{&is_parameterless}
    {

    }

    FunctionQuery::FunctionQuery(const vector<TypePtr>& return_types, const string& name, const TypePtr& template_type, const ArgumentList& args, const bool& is_parameterless)
        : return_types{&return_types}, name{&name}, template_type{&template_type}, args{&args}, is_parameterless{&is_parameterless}
    {

    }

    FunctionQuery::FunctionQuery(const TypePtr& class_type,const vector<TypePtr>& return_types, const string& name, const bool& is_parameterless)
        : class_type{&class_type}, return_types{&return_types}, name{&name}, is_parameterless{&is_parameterless}
    {

    }

    FunctionQuery::FunctionQuery(
        const TypePtr& class_type,
        const vector<TypePtr>& return_types,
        const string& name,
        const TypePtr& template_type,
        const ArgumentList& args,
        const bool& is_parameterless
    ) : class_type{&class_type},
        return_types{&return_types},
        name{&name},
        template_type{&template_type},
        args{&args},
        is_parameterless{&is_parameterless}
    {

    }

    FunctionQuery::Result FunctionQuery::is_match(const FuncPtr& func) const
    {
        const Result class_type_result = class_type_matches(func);
        const Result return_type_result = return_type_matches(func);
        const Result name_result = name_matches(func);
        const Result template_type_result = template_type_matches(func);
        const Result arguments_result = arguments_match(func);
        const Result is_parameterless_result = is_parameterless_matches(func);
        const Result final_result = class_type_result & return_type_result & name_result & template_type_result & arguments_result & is_parameterless_result;
        return final_result;
    }

    FunctionQuery::Result FunctionQuery::has_match(const vector<FuncPtr>& funcs) const
    {
        Result result = Result::NoMatch;
        for (const FuncPtr& func : funcs)
            result |= is_match(func);
        return result;
    }

    FuncPtr FunctionQuery::get_match(const vector<FuncPtr>& funcs, const bool throw_on_fail) const
    {
        vector<FuncPtr> matches = get_matches(funcs);
        if (matches.empty())
        {
            if (throw_on_fail)
                throw CompileError{"No matches found"};
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
                    throw CompileError{"Too many matches found"};
                else
                    return nullptr;
            }
        }
    }

    vector<FuncPtr> FunctionQuery::get_matches(const vector<FuncPtr>& funcs) const
    {
        Result best_result = Result::Default;
        vector<FuncPtr> matches;
        for (const FuncPtr& func : funcs)
        {
            Result result = is_match(func);
            if (result > best_result)
            {
                best_result = result;
                matches.clear();
            }
            if (result == best_result)
            {
                matches.push_back(func);
            }
        }
        return matches;
    }

    FunctionQuery::Result FunctionQuery::class_type_matches(const FuncPtr& func) const
    {
        if (class_type == nullptr)
            return Result::Ignore;
        return func->class_type() == *class_type;
    }

    FunctionQuery::Result FunctionQuery::return_type_matches(const FuncPtr& func) const
    {
        // empty means any type
        if (return_types == nullptr or return_types->empty() or (return_types->size() == 1 and (*return_types)[0]->is_auto()))
            return Result::Ignore;
        if (func->return_type()->is_in(*return_types))
            return Result{Result::Match, 1};
        if (func->return_type()->is_compatible(*return_types))
            return Result::Match;
        return Result::NoMatch;
    }

    FunctionQuery::Result FunctionQuery::name_matches(const FuncPtr& func) const
    {
        if (name == nullptr)
            return Result::Ignore;
        return func->name() == *name;
    }

    FunctionQuery::Result FunctionQuery::template_type_matches(const FuncPtr& func) const
    {
        // nullptr means any type
        if (template_type == nullptr or *template_type == nullptr)
            return Result::Ignore;
        return func->template_type() == *template_type;
    }

    FunctionQuery::Result FunctionQuery::arguments_match(const FuncPtr& func) const
    {
        if (args == nullptr)
            return Result::Ignore;

        if (args->size() > func->max_arity() or args->size() < func->min_arity())
            return Result::NoMatch;

        Result result = Result::Match;
        const ParameterList& params = func->parameters();
        for (const Argument& arg : *args)
        {
            if (not params.contains(arg))
                return Result::NoMatch;
            if (not arg.is_initialized())
                continue;
            const TypePtr param_type = params[arg].type();
            if (not param_type->is_compatible(arg.type()))
                return Result::NoMatch;
            if (param_type->equals(arg.type()))
                result.score++;
        }

        return result;
    }

    FunctionQuery::Result FunctionQuery::is_parameterless_matches(const FuncPtr& func) const
    {
        if (is_parameterless == nullptr)
            return Result::Ignore;
        return func->is_parameterless() == *is_parameterless;
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
}
