//
// Created by jaket on 26/08/2026.
//

#include "utils/primitive_utils.h"

namespace mxslc::primitive_utils
{
    namespace
    {
        template <typename T>
        T fract_vec(const Primitive& in)
        {
            float int_;

            T v = in.as<T>();
            for (size_t i = 0; i < T::numElements(); i++)
            {
                v[i] = std::modf(v[i], &int_);
            }
            return v;
        }
    }

    Primitive fract(const Primitive& in)
    {
        if (in.is_a<float>())
        {
            float int_;
            return std::modf(in.as<float>(), &int_);
        }

#define FRACT_VEC_IF(type) if (in.is_a<type>()) { return fract_vec<type>(in); }
        FRACT_VEC_IF(mx::Vector2)
        FRACT_VEC_IF(mx::Vector3)
        FRACT_VEC_IF(mx::Vector4)
        FRACT_VEC_IF(mx::Color3)
        FRACT_VEC_IF(mx::Color4)
#undef FRACT_VEC_IF

        throw CompileError{"Cannot fract a value of type '" + in.type_name() + "'"};
    }
}
