//
// Created by jaket on 09/08/2026.
//

#include "utils/primitive_utils.h"
#include "runtime/Type.h"

namespace mxslc::primitive_utils
{
    namespace
    {
        template<typename T>
        T round_vec(const Primitive& in)
        {
            T v = in.as<T>();
            for (size_t i = 0; i < T::numElements(); i++)
            {
                v[i] = std::round(v[i]);
            }
            return v;
        }
    }

    Primitive round(const Primitive& in, const TypePtr& type)
    {
        if (in.is_a<int>() or in.is_a<float>())
        {
            const float out = std::round(in.cast<float>());
            if (type and type->is<int>())
                return static_cast<int>(out);
            return out;
        }

#define ROUND_VEC_IF(type) if (in.is_a<type>()) { return round_vec<type>(in); }
        ROUND_VEC_IF(mx::Vector2)
        ROUND_VEC_IF(mx::Vector3)
        ROUND_VEC_IF(mx::Vector4)
        ROUND_VEC_IF(mx::Color3)
        ROUND_VEC_IF(mx::Color4)
#undef ROUND_VEC_IF

        throw CompileError{"Cannot round a value of type '" + in.type_name() + "'"};
    }
}
