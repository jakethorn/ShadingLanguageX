//
// Created by jaket on 31/07/2026.
//

#include "utils/primitive_utils.h"
#include "runtime/Type.h"

namespace mxslc::primitive_utils
{
    namespace
    {
        template<typename T>
        Primitive invert_vec(const Primitive& in, const Primitive& amount)
        {
            const T v = in.as<T>();
            if (amount.is_a<int>() or amount.is_a<float>())
            {
                const float a = amount.cast<float>();
                T out;
                for (size_t i = 0; i < T::numElements(); ++i)
                    out[i] = a - v[i];
                return out;
            }

            if (amount.is_a<T>())
            {
                const T a = amount.as<T>();
                T out;
                for (size_t i = 0; i < T::numElements(); ++i)
                    out[i] = a[i] - v[i];
                return out;
            }

            throw CompileError{"Cannot invert a value of type '" + in.type_name() + "'"};
        }
    }

    // MaterialX invert computes: out = amount - in1
    Primitive invert(const Primitive& in1, const Primitive& amount)
    {
        // scalar: amount - in1
        if ((in1.is_a<int>() or in1.is_a<float>()) and (amount.is_a<int>() or amount.is_a<float>()))
            return amount.cast<float>() - in1.cast<float>();

#define INVERT_VEC_IF(type) if (in1.is_a<type>()) { return invert_vec<type>(in1, amount); }
        INVERT_VEC_IF(mx::Vector2)
        INVERT_VEC_IF(mx::Vector3)
        INVERT_VEC_IF(mx::Vector4)
        INVERT_VEC_IF(mx::Color3)
        INVERT_VEC_IF(mx::Color4)
#undef INVERT_VEC_IF

        throw CompileError{"Cannot invert a value of type '" + in1.type_name() + "'"};
    }
}
