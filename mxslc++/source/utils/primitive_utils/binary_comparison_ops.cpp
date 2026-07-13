//
// Created by jaket on 11/07/2026.
//

#include "utils/primitive_utils.h"
#include "runtime/Type.h"

#define COMPARISON_OP_IF(type, op) \
    if ((in1.is_a<type>() and in2.is_castable_to<type>()) or (in2.is_a<type>() and in1.is_castable_to<type>())) \
        return Primitive{in1.cast<type>() op in2.cast<type>()}.cast<bool>();

#define TO_STRING_HELPER(x) #x
#define TO_STRING(x) TO_STRING_HELPER(x)

namespace mxslc::primitive_utils
{
    Primitive equal(const Primitive& in1, const Primitive& in2)
    {
#define OP ==
        COMPARISON_OP_IF(float, OP)
        COMPARISON_OP_IF(int, OP)
        COMPARISON_OP_IF(bool, OP)
        COMPARISON_OP_IF(mx::Vector2, OP)
        COMPARISON_OP_IF(mx::Vector3, OP)
        COMPARISON_OP_IF(mx::Vector4, OP)
        COMPARISON_OP_IF(mx::Color3, OP)
        COMPARISON_OP_IF(mx::Color4, OP)
        COMPARISON_OP_IF(mx::Matrix33, OP)
        COMPARISON_OP_IF(mx::Matrix44, OP)
        COMPARISON_OP_IF(string, OP)
        COMPARISON_OP_IF(fs::path, OP)

        throw CompileError{"Binary operator " TO_STRING(OP) " not supported for values of type '" + in1.type_name() + "' and '" + in2.type_name() + "'"};
#undef OP
    }

    Primitive not_equal(const Primitive& in1, const Primitive& in2)
    {
#define OP !=
        COMPARISON_OP_IF(float, OP)
        COMPARISON_OP_IF(int, OP)
        COMPARISON_OP_IF(bool, OP)
        COMPARISON_OP_IF(mx::Vector2, OP)
        COMPARISON_OP_IF(mx::Vector3, OP)
        COMPARISON_OP_IF(mx::Vector4, OP)
        COMPARISON_OP_IF(mx::Color3, OP)
        COMPARISON_OP_IF(mx::Color4, OP)
        COMPARISON_OP_IF(mx::Matrix33, OP)
        COMPARISON_OP_IF(mx::Matrix44, OP)
        COMPARISON_OP_IF(string, OP)
        COMPARISON_OP_IF(fs::path, OP)

        throw CompileError{"Binary operator " TO_STRING(OP) " not supported for values of type '" + in1.type_name() + "' and '" + in2.type_name() + "'"};
#undef OP
    }

    Primitive greater(const Primitive& in1, const Primitive& in2)
    {
#define OP >
        COMPARISON_OP_IF(float, OP)
        COMPARISON_OP_IF(int, OP)
        COMPARISON_OP_IF(bool, OP)

        throw CompileError{"Binary operator " TO_STRING(OP) " not supported for values of type '" + in1.type_name() + "' and '" + in2.type_name() + "'"};
#undef OP
    }

    Primitive less(const Primitive& in1, const Primitive& in2)
    {
#define OP <
        COMPARISON_OP_IF(float, OP)
        COMPARISON_OP_IF(int, OP)
        COMPARISON_OP_IF(bool, OP)

        throw CompileError{"Binary operator " TO_STRING(OP) " not supported for values of type '" + in1.type_name() + "' and '" + in2.type_name() + "'"};
#undef OP
    }

    Primitive greater_equal(const Primitive& in1, const Primitive& in2)
    {
#define OP >=
        COMPARISON_OP_IF(float, OP)
        COMPARISON_OP_IF(int, OP)
        COMPARISON_OP_IF(bool, OP)

        throw CompileError{"Binary operator " TO_STRING(OP) " not supported for values of type '" + in1.type_name() + "' and '" + in2.type_name() + "'"};
#undef OP
    }

    Primitive less_equal(const Primitive& in1, const Primitive& in2)
    {
#define OP <=
        COMPARISON_OP_IF(float, OP)
        COMPARISON_OP_IF(int, OP)
        COMPARISON_OP_IF(bool, OP)

        throw CompileError{"Binary operator " TO_STRING(OP) " not supported for values of type '" + in1.type_name() + "' and '" + in2.type_name() + "'"};
#undef OP
    }
}

#undef TO_STRING
#undef TO_STRING_HELPER
#undef COMPARISON_OP_IF
