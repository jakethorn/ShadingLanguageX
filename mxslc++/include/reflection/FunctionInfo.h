//
// Created by jaket on 19/11/2026.
//

#ifndef MXSLC_FUNCTIONINFO_H
#define MXSLC_FUNCTIONINFO_H

#include "common.h"
#include "OverloadInfo.h"

namespace mxslc::reflection
{
    struct FunctionInfo
    {
        string name;
        vector<OverloadInfo> overloads;
    };
}

#endif //MXSLC_FUNCTIONINFO_H
