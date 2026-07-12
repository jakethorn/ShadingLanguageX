//
// Created by jaket on 12/07/2026.
//

#include "primitive/primitive_utils.h"
#include "runtime/Type.h"

namespace mxslc::primitive_utils
{
    std::array<Primitive, 2> separate2(const Primitive& in)
    {
        if (in.is_a<mx::Vector2>())
            return {in[0], in[1]};

        throw CompileError{"Cannot separate2 a value of type '" + in.type()->name() + "'"};
    }

    std::array<Primitive, 3> separate3(const Primitive& in)
    {
        if (in.is_a<mx::Vector3>() or in.is_a<mx::Color3>())
            return {in[0], in[1], in[2]};

        throw CompileError{"Cannot separate3 a value of type '" + in.type()->name() + "'"};
    }

    std::array<Primitive, 4> separate4(const Primitive& in)
    {
        if (in.is_a<mx::Vector4>() or in.is_a<mx::Color4>())
            return {in[0], in[1], in[2], in[3]};

        throw CompileError{"Cannot separate4 a value of type '" + in.type()->name() + "'"};
    }

    vector<Primitive> separate(const Primitive& in)
    {
        if (in.is_a<float>())
            return {in};
        if (in.is_a<mx::Vector2>())
            return {in[0], in[1]};
        if (in.is_a<mx::Vector3>() or in.is_a<mx::Color3>())
            return {in[0], in[1], in[2]};
        if (in.is_a<mx::Vector4>() or in.is_a<mx::Color4>())
            return {in[0], in[1], in[2], in[3]};

        throw CompileError{"Cannot separate a value of type '" + in.type()->name() + "'"};
    }
}
