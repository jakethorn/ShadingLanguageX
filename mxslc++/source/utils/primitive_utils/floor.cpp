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
        T floor_vec(const Primitive& in)
        {
            T v = in.as<T>();
            for (size_t i = 0; i < T::numElements(); i++)
            {
                v[i] = std::floor(v[i]);
            }
            return v;
        }
    }

    Primitive floor(const Primitive& in, const TypePtr& type)
    {
        if (in.is_a<int>() or in.is_a<float>())
        {
            const float out = std::floor(in.cast<float>());
            if (type and type->is<int>())
                return static_cast<int>(out);
            return out;
        }

#define FLOOR_VEC_IF(type) if (in.is_a<type>()) { return floor_vec<type>(in); }
        FLOOR_VEC_IF(mx::Vector2)
        FLOOR_VEC_IF(mx::Vector3)
        FLOOR_VEC_IF(mx::Vector4)
        FLOOR_VEC_IF(mx::Color3)
        FLOOR_VEC_IF(mx::Color4)
#undef FLOOR_VEC_IF

        throw CompileError{"Cannot floor a value of type '" + in.type_name() + "'"};
    }
}
