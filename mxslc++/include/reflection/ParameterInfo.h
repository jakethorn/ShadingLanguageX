//
// Created by jaket on 20/11/2026.
//

#ifndef MXSLC_PARAMETERINFO_H
#define MXSLC_PARAMETERINFO_H

#include "common.h"

namespace mxslc::reflection
{
    struct ParameterInfo
    {
        string name;
        string type;
        string default_expression;
        vector<string> modifiers;
    };
}

#endif //MXSLC_PARAMETERINFO_H
