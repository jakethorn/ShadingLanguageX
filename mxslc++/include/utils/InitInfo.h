//
// Created by jaket on 26/06/2026.
//

#ifndef MXSLC_INITINFO_H
#define MXSLC_INITINFO_H

#include "utils/common.h"

struct InitInfo
{
    // Types the expression can evaluate to
    vector<TypePtr> types;

    // Types the expression can evaluate to many of
    vector<TypePtr> sub_types;

    // A specific index to look at when considering the sub_type
    int index;

    // A specific property to look at when considering the sub_type
    string property;
};

#endif //MXSLC_INITINFO_H
