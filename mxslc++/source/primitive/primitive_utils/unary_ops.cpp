//
// Created by jaket on 11/07/2026.
//

#include "primitive/primitive_utils.h"
#include "runtime/Type.h"

#define BINARY_OP_IF(type, op) \
    if ((in1.is_a<type>() and in2.is_castable_to<type>()) or (in2.is_a<type>() and in1.is_castable_to<type>())) \
        return Primitive{in1.cast<type>() op in2.cast<type>()}.cast<type>();

namespace mxslc::primitive_utils
{
    Primitive positive(const Primitive& in)
    {
#define UNARY_PLUS_IF(type) if (in.is_a<type>()) { return in; }
        UNARY_PLUS_IF(float)
        UNARY_PLUS_IF(int)
        UNARY_PLUS_IF(bool)
        UNARY_PLUS_IF(mx::Vector2)
        UNARY_PLUS_IF(mx::Vector3)
        UNARY_PLUS_IF(mx::Vector4)
        UNARY_PLUS_IF(mx::Color3)
        UNARY_PLUS_IF(mx::Color4)
        UNARY_PLUS_IF(mx::Matrix33)
        UNARY_PLUS_IF(mx::Matrix44)
#undef UNARY_PLUS_IF

        throw CompileError{"Unary operator + not supported for value of type '" + in.type()->name() + "'"};
    }

    Primitive negative(const Primitive& in)
    {
#define UNARY_MINUS_IF(type) if (in.is_a<type>()) { return -in.as<type>(); }
        UNARY_MINUS_IF(float)
        UNARY_MINUS_IF(int)
        UNARY_MINUS_IF(bool)
        UNARY_MINUS_IF(mx::Vector2)
        UNARY_MINUS_IF(mx::Vector3)
        UNARY_MINUS_IF(mx::Vector4)
        UNARY_MINUS_IF(mx::Color3)
        UNARY_MINUS_IF(mx::Color4)
#undef UNARY_MINUS_IF

        if (in.is_a<mx::Matrix33>())
            return mx::Matrix33{0.0f} - in.as<mx::Matrix33>();

        if (in.is_a<mx::Matrix44>())
            return mx::Matrix44{0.0f} - in.as<mx::Matrix44>();

        throw CompileError{"Unary operator - not supported for value of type '" + in.type()->name() + "'"};
    }

    Primitive logical_not(const Primitive& in)
    {
#define UNARY_BANG_IF(type) if (in.is_a<type>()) { return !in.as<type>(); }
        UNARY_BANG_IF(float)
        UNARY_BANG_IF(int)
        UNARY_BANG_IF(bool)
#undef UNARY_BANG_IF

        throw CompileError{"Unary operator ! not supported for value of type '" + in.type()->name() + "'"};
    }
}

#undef BINARY_OP_IF
