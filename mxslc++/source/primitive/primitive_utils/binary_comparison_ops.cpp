//
// Created by jaket on 11/07/2026.
//

#include "primitive/primitive_utils.h"
#include "runtime/Type.h"

#define BINARY_OP_IF(type, op) \
    if ((in1.is_a<type>() and in2.is_castable_to<type>()) or (in2.is_a<type>() and in1.is_castable_to<type>())) \
        return Primitive{in1.cast<type>() op in2.cast<type>()}.cast<bool>();

namespace mxslc::primitive_utils
{
    Primitive equal(const Primitive& in1, const Primitive& in2)
    {
        BINARY_OP_IF(float, ==)
        BINARY_OP_IF(int, ==)
        BINARY_OP_IF(bool, ==)
        BINARY_OP_IF(mx::Vector2, ==)
        BINARY_OP_IF(mx::Vector3, ==)
        BINARY_OP_IF(mx::Vector4, ==)
        BINARY_OP_IF(mx::Color3, ==)
        BINARY_OP_IF(mx::Color4, ==)
        BINARY_OP_IF(mx::Matrix33, ==)
        BINARY_OP_IF(mx::Matrix44, ==)
        BINARY_OP_IF(string, ==)

        throw CompileError{"Binary operator == not supported for values of type '" + in1.type()->name() + "' and '" + in2.type()->name() + "'"};
    }

    Primitive not_equal(const Primitive& in1, const Primitive& in2)
    {
        BINARY_OP_IF(float, !=)
        BINARY_OP_IF(int, !=)
        BINARY_OP_IF(bool, !=)
        BINARY_OP_IF(mx::Vector2, !=)
        BINARY_OP_IF(mx::Vector3, !=)
        BINARY_OP_IF(mx::Vector4, !=)
        BINARY_OP_IF(mx::Color3, !=)
        BINARY_OP_IF(mx::Color4, !=)
        BINARY_OP_IF(mx::Matrix33, !=)
        BINARY_OP_IF(mx::Matrix44, !=)
        BINARY_OP_IF(string, !=)

        throw CompileError{"Binary operator != not supported for values of type '" + in1.type()->name() + "' and '" + in2.type()->name() + "'"};
    }

    Primitive greater(const Primitive& in1, const Primitive& in2)
    {
        BINARY_OP_IF(float, >)
        BINARY_OP_IF(int, >)
        BINARY_OP_IF(bool, >)

        throw CompileError{"Binary operator > not supported for values of type '" + in1.type()->name() + "' and '" + in2.type()->name() + "'"};
    }

    Primitive less(const Primitive& in1, const Primitive& in2)
    {
        BINARY_OP_IF(float, <)
        BINARY_OP_IF(int, <)
        BINARY_OP_IF(bool, <)

        throw CompileError{"Binary operator < not supported for values of type '" + in1.type()->name() + "' and '" + in2.type()->name() + "'"};
    }

    Primitive greater_equal(const Primitive& in1, const Primitive& in2)
    {
        BINARY_OP_IF(float, >=)
        BINARY_OP_IF(int, >=)
        BINARY_OP_IF(bool, >=)

        throw CompileError{"Binary operator >= not supported for values of type '" + in1.type()->name() + "' and '" + in2.type()->name() + "'"};
    }

    Primitive less_equal(const Primitive& in1, const Primitive& in2)
    {
        BINARY_OP_IF(float, <=)
        BINARY_OP_IF(int, <=)
        BINARY_OP_IF(bool, <=)

        throw CompileError{"Binary operator <= not supported for values of type '" + in1.type()->name() + "' and '" + in2.type()->name() + "'"};
    }
}

#undef BINARY_OP_IF
