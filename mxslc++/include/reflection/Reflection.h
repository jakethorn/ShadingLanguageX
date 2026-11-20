//
// Created by jaket on 04/11/2026.
//

#ifndef MXSLC_REFLECTION_H
#define MXSLC_REFLECTION_H

#include "common.h"
#include "GlobalInfo.h"
#include "TypeInfo.h"
#include "FunctionInfo.h"
#include "runtime/utils/RuntimeAware.h"

namespace mxslc::reflection
{
    class Reflection : protected runtime_utils::RuntimeAware
    {
    public:
        void add_global(string name, TypePtr type, VarPtr value);
        void collect_info();

        const vector<GlobalInfo>& globals() const { return globals_; }
        const vector<TypeInfo>& types() const { return types_; }
        const vector<FunctionInfo>& functions() const { return functions_; }

    private:

        vector<GlobalInfo> globals_;
        vector<TypeInfo> types_;
        vector<FunctionInfo> functions_;
    };
}

#endif //MXSLC_REFLECTION_H
