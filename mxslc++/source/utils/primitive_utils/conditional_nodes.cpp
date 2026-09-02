//
// Created by jaket on 16/07/2026.
//

#include "utils/primitive_utils.h"

namespace mxslc::primitive_utils
{
    Primitive ifequal(const Primitive& value1, const Primitive& value2)
    {
        return equal(value1, value2);
    }

    Primitive ifequal(const Primitive& value1, const Primitive& value2, const Primitive& in1, const Primitive& in2)
    {
        return equal(value1, value2) ? in1 : in2;
    }

    Primitive switch_(const vector<Primitive>& ins, const Primitive& which, const TypePtr& type)
    {
        const int i = which.is_null() ? 0 : which.cast<int>();

        if (i >= ins.size())
            throw CompileError{"Index out of bounds in switch"};

        Primitive result = ins[i];

        if (not result.is_null())
            return result;

        if (type)
            return Primitive{type};

        for (const auto& in : ins)
            if (not in.is_null())
                return Primitive{in.type()};

        throw CompileError{"Cannot determine return type of switch"};
    }
}
