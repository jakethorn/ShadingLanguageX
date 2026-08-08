//
// Created by jaket on 27/06/2026.
//

#ifndef MXSLC_ARGUMENTEVALUATOR_H
#define MXSLC_ARGUMENTEVALUATOR_H

#include "RuntimeAware.h"
#include "runtime/FunctionQuery.h"

namespace mxslc::runtime
{
    class Argument;
}

namespace mxslc::runtime_utils
{
    using runtime_utils::RuntimeAware;

    class ArgumentEvaluator : protected RuntimeAware
    {
    public:
        explicit ArgumentEvaluator(const FunctionQuery& query) : query_{query} { }

        FuncPtr execute(ArgumentList& args) const;

    private:
        FuncPtr try_init(ArgumentList& args, const Scope*& scope, vector<FuncPtr>& checked_funcs) const;

        size_t count_initialized(const ArgumentList& args) const;
        size_t try_init(const FuncPtr& func, ArgumentList& args) const;
        size_t try_init(const vector<FuncPtr>& funcs, ArgumentList& args) const;
        size_t try_init_with_default(const vector<FuncPtr>& funcs, ArgumentList& args) const;
        vector<TypePtr> get_target_types(const vector<FuncPtr>& funcs, const Argument& arg) const;

        FunctionQuery query_;
    };
}

#endif //MXSLC_ARGUMENTEVALUATOR_H
