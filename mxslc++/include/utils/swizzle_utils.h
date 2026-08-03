//
// Created by jaket on 01/08/2026.
//

#ifndef MXSLC_SWIZZLE_UTILS_H
#define MXSLC_SWIZZLE_UTILS_H

#include "common.h"

namespace mxslc::swizzle_utils
{
    VarPtr swizzle(const VarPtr& value, const string& swizzle);

    TypePtr get_swizzle_type(const string& swizzle);
    int get_channel_index(char channel);

    bool is_swizzle(const string& swizzle);
    bool is_valid_swizzle(const TypePtr& type, const string& swizzle);
    bool has_duplicate_channels(const string& swizzle);
}

#endif //MXSLC_SWIZZLE_UTILS_H
