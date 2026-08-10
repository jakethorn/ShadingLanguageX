//
// Created by jaket on 09/08/2026.
//

#include "utils/primitive_utils.h"

namespace mxslc::primitive_utils
{
    Primitive constant(const Primitive& value, const TypePtr& type)
    {
        if (value.is_null())
            return Primitive{type};
        else
            return value;
    }
}
