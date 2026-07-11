//
// Created by jaket on 11/07/2026.
//

#include "primitive/primitive_utils.h"
#include "runtime/Type.h"

namespace mxslc::primitive_utils
{
    namespace
    {
        template<typename T>
        Primitive extract(const Primitive& in, const Primitive& index)
        {
            const int index_value = index.as<int>();

            int n;
            if constexpr (std::is_base_of_v<mx::VectorBase, T>)
                n = T::numElements();
            else
                n = T::numRows();

            if (index_value >= n)
                throw CompileError{"Index '" + std::to_string(index_value) + "' is out of range for type '" + in.type()->name() + "'"};

            return in.as<T>()[index_value];
        }
    }

    Primitive extract(const Primitive& in, const size_t index)
    {
        return extract(in, Primitive{static_cast<int>(index)});
    }

    Primitive extract(const Primitive& in, const Primitive& index)
    {
#define EXTRACT_IF(from_type) if (in.is_a<from_type>()) { return extract<from_type>(in, index); }
        EXTRACT_IF(mx::Vector2)
        EXTRACT_IF(mx::Vector3)
        EXTRACT_IF(mx::Vector4)
        EXTRACT_IF(mx::Color3)
        EXTRACT_IF(mx::Color4)
        EXTRACT_IF(mx::Matrix33)
        EXTRACT_IF(mx::Matrix44)
#undef EXTRACT_IF

        throw CompileError{"Cannot extract value of type '" + in.type()->name() + "'"};
    }
}
