//
// Created by jaket on 11/07/2026.
//

#include "primitive/primitive_utils.h"
#include "runtime/Type.h"

#define ARITHMATIC_OP_IF(type, op) \
    if ((in1.is_a<type>() and in2.is_castable_to<type>()) or (in2.is_a<type>() and in1.is_castable_to<type>())) \
        return Primitive{in1.cast<type>() op in2.cast<type>()}.cast<type>();

#define TO_STRING_HELPER(x) #x
#define TO_STRING(x) TO_STRING_HELPER(x)

namespace mxslc::primitive_utils
{
    Primitive add(const Primitive& in1, const Primitive& in2)
    {
#define OP +
        ARITHMATIC_OP_IF(float, OP)
        ARITHMATIC_OP_IF(int, OP)
        ARITHMATIC_OP_IF(bool, OP)
        ARITHMATIC_OP_IF(mx::Vector2, OP)
        ARITHMATIC_OP_IF(mx::Vector3, OP)
        ARITHMATIC_OP_IF(mx::Vector4, OP)
        ARITHMATIC_OP_IF(mx::Color3, OP)
        ARITHMATIC_OP_IF(mx::Color4, OP)
        ARITHMATIC_OP_IF(mx::Matrix33, OP)
        ARITHMATIC_OP_IF(mx::Matrix44, OP)
        ARITHMATIC_OP_IF(string, OP)

        throw CompileError{"Binary operator " TO_STRING(OP) " not supported for values of type '" + in1.type_name() + "' and '" + in2.type_name() + "'"};
#undef OP
    }

    Primitive subtract(const Primitive& in1, const Primitive& in2)
    {
#define OP -
        ARITHMATIC_OP_IF(float, -)
        ARITHMATIC_OP_IF(int, -)
        ARITHMATIC_OP_IF(bool, -)
        ARITHMATIC_OP_IF(mx::Vector2, -)
        ARITHMATIC_OP_IF(mx::Vector3, -)
        ARITHMATIC_OP_IF(mx::Vector4, -)
        ARITHMATIC_OP_IF(mx::Color3, -)
        ARITHMATIC_OP_IF(mx::Color4, -)
        ARITHMATIC_OP_IF(mx::Matrix33, -)
        ARITHMATIC_OP_IF(mx::Matrix44, -)

        throw CompileError{"Binary operator " TO_STRING(OP) " not supported for values of type '" + in1.type_name() + "' and '" + in2.type_name() + "'"};
#undef OP
    }

    Primitive multiply(const Primitive& in1, const Primitive& in2)
    {
#define OP *
        ARITHMATIC_OP_IF(float, OP)
        ARITHMATIC_OP_IF(int, OP)
        ARITHMATIC_OP_IF(bool, OP)
        ARITHMATIC_OP_IF(mx::Vector2, OP)
        ARITHMATIC_OP_IF(mx::Vector3, OP)
        ARITHMATIC_OP_IF(mx::Vector4, OP)
        ARITHMATIC_OP_IF(mx::Color3, OP)
        ARITHMATIC_OP_IF(mx::Color4, OP)
        ARITHMATIC_OP_IF(mx::Matrix33, OP)
        ARITHMATIC_OP_IF(mx::Matrix44, OP)

        throw CompileError{"Binary operator " TO_STRING(OP) " not supported for values of type '" + in1.type_name() + "' and '" + in2.type_name() + "'"};
#undef OP
    }

    Primitive divide(const Primitive& in1, const Primitive& in2)
    {
#define OP /
        ARITHMATIC_OP_IF(float, OP)
        ARITHMATIC_OP_IF(int, OP)
        ARITHMATIC_OP_IF(bool, OP)
        ARITHMATIC_OP_IF(mx::Vector2, OP)
        ARITHMATIC_OP_IF(mx::Vector3, OP)
        ARITHMATIC_OP_IF(mx::Vector4, OP)
        ARITHMATIC_OP_IF(mx::Color3, OP)
        ARITHMATIC_OP_IF(mx::Color4, OP)
        ARITHMATIC_OP_IF(mx::Matrix33, OP)
        ARITHMATIC_OP_IF(mx::Matrix44, OP)
        ARITHMATIC_OP_IF(fs::path, OP)

        if (in1.is_a<string>() and in2.is_a<string>())
            return Primitive{in1.cast<fs::path>() / in2.cast<fs::path>()}.cast<string>();

        throw CompileError{"Binary operator " TO_STRING(OP) " not supported for values of type '" + in1.type_name() + "' and '" + in2.type_name() + "'"};
#undef OP
    }
}

#undef TO_STRING
#undef TO_STRING_HELPER
#undef ARITHMATIC_OP_IF
