//
// Created by jaket on 12/06/2026.
//

#ifndef MXSLC_INVOKE_UTILS_H
#define MXSLC_INVOKE_UTILS_H

#include "common.h"
#include "runtime/ArgumentList.h"

namespace mxslc::runtime_utils
{
    VarPtr invoke_function(string func_name, ArgumentList args);
    VarPtr invoke_function(const TypePtr& target_type, string func_name, ArgumentList args);
    VarPtr invoke_function(const vector<TypePtr>& target_types, string func_name);

    VarPtr invoke_method(const vector<TypePtr>& target_types, VarPtr instance, string func_name);

    VarPtr invoke_constructor(string ctor_name, ArgumentList args);
    VarPtr invoke_constructor(const TypePtr& target_type, string ctor_name, ArgumentList args);
    VarPtr invoke_constructor(const TypePtr& type, ArgumentList args);
}

#endif //MXSLC_INVOKE_UTILS_H
