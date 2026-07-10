//
// Created by jaket on 10/07/2026.
//

#ifndef MXSLC_TYPE_UTILS_H
#define MXSLC_TYPE_UTILS_H

#include "common.h"

namespace mxslc::type_utils
{
    bool contains_auto(const TypePtr& type);
    TypePtr replace_auto(const TypePtr& original_type, const TypePtr& replacement_type);

    TypePtr type_of(const vector<VarPtr>& children);
}

#endif //MXSLC_TYPE_UTILS_H
