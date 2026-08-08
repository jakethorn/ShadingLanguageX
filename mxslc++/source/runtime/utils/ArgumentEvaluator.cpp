//
// Created by jaket on 27/06/2026.
//

#include "runtime/utils/ArgumentEvaluator.h"

#include "errors/AmbiguousFunctionError.h"
#include "runtime/ArgumentList.h"
#include "runtime/Function.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"

namespace mxslc::runtime_utils
{
    using container_utils::extend;

    FuncPtr ArgumentEvaluator::execute(ArgumentList& args) const
    {
        const Scope* scope_ = &scope();
        vector<FuncPtr> checked_funcs;
        while (scope_ != nullptr)
        {
            FuncPtr func = try_init(args, scope_, checked_funcs);
            if (func != nullptr)
                return func;
        }

        throw AmbiguousFunctionError{*query_.name, checked_funcs};
    }

    FuncPtr ArgumentEvaluator::try_init(ArgumentList& args, const Scope*& scope, vector<FuncPtr>& checked_funcs) const
    {
        size_t init_count = count_initialized(args);
        while (init_count < args.size())
        {
            size_t last_init_count = init_count;
            vector<FuncPtr> funcs = scope->get_functions(query_);
            init_count = try_init(funcs, args);

            if (last_init_count == init_count)
            {
                init_count = try_init_with_default(funcs, args);
                if (init_count < args.size())
                {
                    scope = funcs[0]->defining_scope()->parent();
                    extend(checked_funcs, std::move(funcs));
                    return nullptr;
                }
            }
        }

        return scope->get_function(query_);
    }

    size_t ArgumentEvaluator::try_init(const FuncPtr& func, ArgumentList& args) const
    {
        return try_init(vector{func}, args);
    }

    size_t ArgumentEvaluator::try_init(const vector<FuncPtr>& funcs, ArgumentList& args) const
    {
        size_t initialized_arg_count = 0;

        for (Argument& arg : args)
        {
            vector<TypePtr> target_types = get_target_types(funcs, arg);
            if (arg.is_initialized() or arg.try_init(target_types))
                initialized_arg_count++;
        }
        return initialized_arg_count;
    }

    size_t ArgumentEvaluator::try_init_with_default(const vector<FuncPtr>& funcs, ArgumentList& args) const
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

        return try_init(default_func, args);
    }

    vector<TypePtr> ArgumentEvaluator::get_target_types(const vector<FuncPtr>& funcs, const Argument& arg) const
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
