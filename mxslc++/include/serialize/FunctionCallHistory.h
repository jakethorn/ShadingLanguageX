//
// Created by jaket on 05/09/2026.
//

#ifndef MXSLC_FUNCTIONCALLHISTORY_H
#define MXSLC_FUNCTIONCALLHISTORY_H

#include "common.h"
#include "runtime/interface.h"
#include "expressions/interface.h"

namespace mxslc::serialize
{
    class FunctionCallHistory
    {
    public:
        FunctionCallHistory() = default;

        void add_function(ConstFuncPtr func);
        void add_call(ConstFunctionCallPtr call);

        const unordered_map<ConstFuncPtr, vector<ConstFunctionCallPtr>>& calls() const { return calls_; }

        size_t size() const { return calls_.size(); }
        bool empty() const { return calls_.empty(); }

        const vector<ConstFunctionCallPtr>& operator[](const ConstFuncPtr& func) const { return calls_.at(func); }

        auto begin() { return calls_.begin(); }
        auto begin() const { return calls_.begin(); }

        auto end() { return calls_.end(); }
        auto end() const { return calls_.end(); }

    private:
        unordered_map<ConstFuncPtr, vector<ConstFunctionCallPtr>> calls_;
    };
}

#endif //MXSLC_FUNCTIONCALLHISTORY_H
