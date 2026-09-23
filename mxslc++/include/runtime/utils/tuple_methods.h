//
// Created by jaket on 19/09/2026.
//

#ifndef MXSLC_TUPLE_METHODS_H
#define MXSLC_TUPLE_METHODS_H

#include "common.h"

namespace mxslc::runtime
{
    class ArgumentList;
}

namespace mxslc::runtime_utils
{
    FuncPtr create_tuple_method(const VarPtr& instance, const string& method_name, const ArgumentList& args);
}

#endif //MXSLC_TUPLE_METHODS_H
