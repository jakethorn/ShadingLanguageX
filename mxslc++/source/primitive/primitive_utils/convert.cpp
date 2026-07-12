//
// Created by jaket on 11/07/2026.
//

#include "primitive/primitive_utils.h"
#include "runtime/Type.h"

#define CONVERT_SINGLE_TO_VECTOR_IF(to_type) if (type->is<to_type>()) { return to_type{in.cast<float>()}; }

#define CONVERT_VECTOR_TO_VECTOR_IF(from_type, to_type) \
    if (in.is_a<from_type>()) \
    { \
        if (type->is<to_type>()) \
        { \
            vector<float> data(to_type::numElements(), 0.0f); \
            for (size_t i = 0; i < std::min(from_type::numElements(), to_type::numElements()); ++i) \
            { \
                data[i] = in.as<from_type>()[i]; \
            } \
            return to_type{data}; \
        } \
    }

namespace mxslc::primitive_utils
{
    Primitive convert(const Primitive& in, const TypePtr& type)
    {
        if (in.type()->equals(type))
            return in;

        if (in.is_a<bool>() or in.is_a<int>() or in.is_a<float>())
        {
            if (type->is<bool>())
                return in.cast<bool>();

            if (type->is<int>())
                return in.cast<int>();

            if (type->is<float>())
                return in.cast<float>();

            CONVERT_SINGLE_TO_VECTOR_IF(mx::Vector2)
            CONVERT_SINGLE_TO_VECTOR_IF(mx::Vector3)
            CONVERT_SINGLE_TO_VECTOR_IF(mx::Vector4)
            CONVERT_SINGLE_TO_VECTOR_IF(mx::Color3)
            CONVERT_SINGLE_TO_VECTOR_IF(mx::Color4)
            CONVERT_SINGLE_TO_VECTOR_IF(mx::Matrix33)
            CONVERT_SINGLE_TO_VECTOR_IF(mx::Matrix44)
        }

        CONVERT_VECTOR_TO_VECTOR_IF(mx::Vector2, mx::Vector3)
        CONVERT_VECTOR_TO_VECTOR_IF(mx::Vector2, mx::Vector4)
        CONVERT_VECTOR_TO_VECTOR_IF(mx::Vector2, mx::Color3)
        CONVERT_VECTOR_TO_VECTOR_IF(mx::Vector2, mx::Color4)

        CONVERT_VECTOR_TO_VECTOR_IF(mx::Vector3, mx::Vector2)
        CONVERT_VECTOR_TO_VECTOR_IF(mx::Vector3, mx::Vector4)
        CONVERT_VECTOR_TO_VECTOR_IF(mx::Vector3, mx::Color3)
        CONVERT_VECTOR_TO_VECTOR_IF(mx::Vector3, mx::Color4)

        CONVERT_VECTOR_TO_VECTOR_IF(mx::Vector4, mx::Vector2)
        CONVERT_VECTOR_TO_VECTOR_IF(mx::Vector4, mx::Vector3)
        CONVERT_VECTOR_TO_VECTOR_IF(mx::Vector4, mx::Color3)
        CONVERT_VECTOR_TO_VECTOR_IF(mx::Vector4, mx::Color4)

        CONVERT_VECTOR_TO_VECTOR_IF(mx::Color3, mx::Vector2)
        CONVERT_VECTOR_TO_VECTOR_IF(mx::Color3, mx::Vector3)
        CONVERT_VECTOR_TO_VECTOR_IF(mx::Color3, mx::Vector4)
        CONVERT_VECTOR_TO_VECTOR_IF(mx::Color3, mx::Color4)

        CONVERT_VECTOR_TO_VECTOR_IF(mx::Color4, mx::Vector2)
        CONVERT_VECTOR_TO_VECTOR_IF(mx::Color4, mx::Vector3)
        CONVERT_VECTOR_TO_VECTOR_IF(mx::Color4, mx::Vector4)
        CONVERT_VECTOR_TO_VECTOR_IF(mx::Color4, mx::Color3)

        if (in.is_a<string>() and type->is<fs::path>())
            return fs::path{in.as<string>()};

        if (in.is_a<fs::path>() and type->is<string>())
            return in.as<fs::path>().string();

        throw CompileError{"Cannot convert a value of type '" + in.type()->name() + "' to '" + type->name() + "'"};
    }
}
