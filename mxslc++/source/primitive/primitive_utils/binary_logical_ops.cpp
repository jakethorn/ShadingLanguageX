//
// Created by jaket on 11/07/2026.
//

#include "primitive/primitive_utils.h"
#include "runtime/Type.h"

namespace mxslc::primitive_utils
{
    Primitive logical_and(const Primitive& in1, const Primitive& in2)
    {
        if (in1.is_castable_to<bool>() and in2.is_castable_to<bool>())
            return in1.cast<bool>() and in2.cast<bool>();

        throw CompileError{"Logical and operator not supported for values of type '" + in1.type()->name() + "' and '" + in2.type()->name() + "'"};
    }

    Primitive logical_or(const Primitive& in1, const Primitive& in2)
    {
        if (in1.is_castable_to<bool>() and in2.is_castable_to<bool>())
            return in1.cast<bool>() or in2.cast<bool>();

        throw CompileError{"Logical or operator not supported for values of type '" + in1.type()->name() + "' and '" + in2.type()->name() + "'"};
    }

    Primitive logical_xor(const Primitive& in1, const Primitive& in2)
    {
        if (in1.is_castable_to<bool>() and in2.is_castable_to<bool>())
            return static_cast<bool>(in1.cast<bool>() xor in2.cast<bool>());

        throw CompileError{"Logical xor operator not supported for values of type '" + in1.type()->name() + "' and '" + in2.type()->name() + "'"};
    }
}
