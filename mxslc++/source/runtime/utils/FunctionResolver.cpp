//
// Created by jaket on 27/06/2026.
//

#include <cassert>

#include "runtime/utils/FunctionResolver.h"

#include "errors/AmbiguousFunctionError.h"
#include "runtime/ArgumentList.h"
#include "runtime/Function.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"

namespace mxslc::runtime_utils
{
    using container_utils::extend;

    FuncPtr resolve_function(const vector<TypePtr>& types, const string& name, const TypePtr& template_type, ArgumentList& args, const bool& is_argumentless)
    {
        const FunctionQuery query{types, name, template_type, args, is_argumentless};
        return FunctionResolver{query, nullptr, args}.resolve();
    }

    FuncPtr resolve_method(const TypePtr& class_type, const vector<TypePtr>& types, const string& name, const TypePtr& template_type, ArgumentList& args, const bool& is_argumentless)
    {
        const FunctionQuery query{class_type, types, name, template_type, args, is_argumentless};
        return FunctionResolver{query, class_type, args}.resolve();
    }

    FuncPtr FunctionResolver::resolve() const
    {
        const Scope* scope_ = &scope();
        vector<FuncPtr> checked_funcs;
        while (scope_ != nullptr)
        {
            FuncPtr func = resolve(scope_, checked_funcs);
            if (func != nullptr)
                return func;
        }

        if (checked_funcs.empty())
            checked_funcs = scope().get_functions(*query_.name);

        if (underlying_errors_.empty())
            throw AmbiguousFunctionError{query_, checked_funcs};
        else
            throw AmbiguousFunctionError{underlying_errors_[0]};
    }

    FuncPtr FunctionResolver::resolve(const Scope*& scope, vector<FuncPtr>& checked_funcs) const
    {
        reset_arguments();

        size_t init_count = 0;
        while (init_count < args_.size())
        {
            size_t last_init_count = init_count;

            vector<FuncPtr> funcs = scope->get_functions(query_);
            if (funcs.empty())
            {
                scope = nullptr;
                return nullptr;
            }

            init_count = init_arguments(funcs);

            if (last_init_count == init_count)
            {
                init_count = init_arguments_with_default_function(funcs);
                if (init_count < args_.size())
                {
                    scope = funcs[0]->defining_scope()->parent();
                    extend(checked_funcs, std::move(funcs));
                    return nullptr;
                }
            }
        }

        FuncPtr func = scope->get_function(query_);
        if (func != nullptr)
        {
            reset_literal_arguments();
            init_count = init_arguments(vector{func});
            assert(init_count == args_.size());
        }

        scope = nullptr;
        return func;
    }

    void FunctionResolver::reset_arguments() const
    {
        for (const Argument& arg : args_)
        {
            if (arg.is_initialized())
                arg.reset();
        }
    }

    void FunctionResolver::reset_literal_arguments() const
    {
        for (const Argument& arg : args_)
        {
            if (arg.is_literal())
                arg.reset();
        }
    }

    size_t FunctionResolver::init_arguments(const vector<FuncPtr>& funcs) const
    {
        size_t initialized_arg_count = 0;

        for (Argument& arg : args_)
        {
            vector<TypePtr> target_types = get_parameter_types(funcs, arg);

            try
            {
                if (arg.is_initialized() or arg.try_init(target_types))
                    initialized_arg_count++;
            }
            catch (const AmbiguousFunctionError& e)
            {
                underlying_errors_.emplace_back(e.message());
            }
        }

        return initialized_arg_count;
    }

    size_t FunctionResolver::init_arguments_with_default_function(const vector<FuncPtr>& funcs) const
    {
        FuncPtr default_func;
        for (const FuncPtr& func : funcs)
        {
            if (func->is_default())
            {
                default_func = func;
                break;
            }
        }

        if (default_func != nullptr)
            return init_arguments(vector{default_func});
        else
            return 0;
    }

    vector<TypePtr> FunctionResolver::get_parameter_types(const vector<FuncPtr>& funcs, const Argument& arg) const
    {
        vector<TypePtr> types;

        for (const FuncPtr& func : funcs)
        {
            const TypePtr& type = func->parameters()[arg].type();
            if (not type->is_in(types))
                types.push_back(type);
        }

        return types;
    }
}
