//
// Created by jaket on 26/08/2026.
//

#include "utils/primitive_utils.h"

#define DOT_IF(type) \
    if (in1.is_a<type>() and in2.is_a<type>()) \
        return in1.as<type>().dot(in2.as<type>());

namespace mxslc::primitive_utils
{
    Primitive dotproduct(const Primitive& in1, const Primitive& in2)
    {
        DOT_IF(mx::Vector2)
        DOT_IF(mx::Vector3)
        DOT_IF(mx::Vector4)

        throw CompileError{"Cannot dotproduct values of type '" + in1.type_name() + "' and '" + in2.type_name() + "'"};
    }
}

#undef DOT_IF
