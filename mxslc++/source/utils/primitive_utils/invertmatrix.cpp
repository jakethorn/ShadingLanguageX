//
// Created by jaket on 16/07/2026.
//

#include "utils/primitive_utils.h"

namespace mxslc::primitive_utils
{
    Primitive invertmatrix(const Primitive& in)
    {
        if (in.is_a<mx::Matrix33>())
            return in.as<mx::Matrix33>().getInverse();
        if (in.is_a<mx::Matrix44>())
            return in.as<mx::Matrix44>().getInverse();

        throw CompileError{"Cannot invert a value of type '" + in.type_name() + "'"};
    }
}
