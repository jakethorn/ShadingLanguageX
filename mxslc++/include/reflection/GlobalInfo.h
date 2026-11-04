//
// Created by jaket on 30/10/2026.
//

#ifndef MXSLC_GLOBALINFO_H
#define MXSLC_GLOBALINFO_H

#include "common.h"

namespace mxslc::reflection
{
    struct GlobalInfo
    {
        string name;
        TypePtr type;
        VarPtr default_value;
    };
}

#endif //MXSLC_GLOBALINFO_H
