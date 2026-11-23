//
// Created by jaket on 04/11/2026.
//

#ifndef MXSLC_REFLECTION_H
#define MXSLC_REFLECTION_H

#include "common.h"
#include "GlobalInfo.h"
#include "runtime/utils/RuntimeAware.h"

namespace mxslc::reflection
{
    class Reflection : protected runtime_utils::RuntimeAware
    {
    public:
        void add_global(string name, TypePtr type, VarPtr value);
        void collect_info();

        const vector<GlobalInfo>& globals() const { return globals_; }
        const vector<string>& type_names() const { return type_names_; }
        const vector<string>& function_names() const { return function_names_; }

    private:
        void collect_type_names();
        void collect_function_names();

        vector<GlobalInfo> globals_;
        vector<string> type_names_;
        vector<string> function_names_;
    };
}

#endif //MXSLC_REFLECTION_H
