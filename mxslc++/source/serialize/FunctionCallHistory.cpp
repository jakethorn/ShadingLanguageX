//
// Created by jaket on 05/09/2026.
//

#include "serialize/FunctionCallHistory.h"

#include "expressions/FunctionCall.h"
#include "utils/container_utils.h"

namespace mxslc::serialize
{
    using container_utils::contains;

    void FunctionCallHistory::add_function(ConstFuncPtr func)
    {
        if (not contains(calls_, func))
            calls_.emplace(std::move(func), vector<ConstFunctionCallPtr>{});
    }

    void FunctionCallHistory::add_call(ConstFunctionCallPtr call)
    {
        if (not contains(calls_, call->function()))
            add_function(call->function());

        calls_[call->function()].push_back(std::move(call));
    }
}
