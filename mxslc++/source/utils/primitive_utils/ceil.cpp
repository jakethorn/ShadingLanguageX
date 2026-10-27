//
// Created by jaket on 27/10/2026.
//

#include "utils/primitive_utils.h"
#include "runtime/Type.h"

namespace mxslc::primitive_utils
{
    namespace
    {
        template<typename T>
        T ceil_vec(const Primitive& in)
        {
            T v = in.as<T>();
            for (size_t i = 0; i < T::numElements(); i++)
            {
                v[i] = std::ceil(v[i]);
            }
            return v;
        }
    }

    Primitive ceil(const Primitive& in, const TypePtr& type)
    {
        if (in.is_a<int>() or in.is_a<float>())
        {
            const float out = std::ceil(in.cast<float>());
            if (type and type->is<int>())
                return static_cast<int>(out);
            return out;
        }

#define CEIL_VEC_IF(type) if (in.is_a<type>()) { return ceil_vec<type>(in); }
        CEIL_VEC_IF(mx::Vector2)
        CEIL_VEC_IF(mx::Vector3)
        CEIL_VEC_IF(mx::Vector4)
        CEIL_VEC_IF(mx::Color3)
        CEIL_VEC_IF(mx::Color4)
#undef CEIL_VEC_IF

        throw CompileError{"Cannot ceil a value of type '" + in.type_name() + "'"};
    }
}
