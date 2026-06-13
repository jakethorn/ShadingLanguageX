//
// Created by jaket on 12/06/2026.
//

#ifndef MXSLC_RUNTIMEUTILS_H
#define MXSLC_RUNTIMEUTILS_H

#include "utils/common.h"

class ArgumentList;

class RuntimeUtils
{
public:
    static VarPtr invoke_function(string func_name, ArgumentList args);
};

#endif //MXSLC_RUNTIMEUTILS_H
