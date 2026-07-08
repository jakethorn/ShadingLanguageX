//
// Created by jaket on 05/07/2026.
//

#include "Primitive.h"
#include "../../include/common.h"

namespace mxslc
{
    string Primitive::str() const
    {
        return std::visit(
            [](const auto& v)
            {
                stringstream ss;

                using ValueType = std::decay_t<decltype(v)>;
                if constexpr (std::is_same_v<ValueType, mx::Vector2>)
                {
                    ss << "vec2{" << v[0] << ", " << v[1] << "}";
                }
                else if constexpr (std::is_same_v<ValueType, mx::Vector3>)
                {
                    ss << "vec3{" << v[0] << ", " << v[1] << ", " << v[2] << "}";
                }
                else if constexpr (std::is_same_v<ValueType, mx::Vector4>)
                {
                    ss << "vec4{" << v[0] << ", " << v[1] << ", " <<  v[2] << ", " << v[3] << "}";
                }
                else if constexpr (std::is_same_v<ValueType, mx::Color3>)
                {
                    ss << "color3{" << v[0] << ", " << v[1] << ", " << v[2] << "}";
                }
                else if constexpr (std::is_same_v<ValueType, mx::Color4>)
                {
                    ss << "Color4{" << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << "}";
                }
                else if constexpr (std::is_same_v<ValueType, mx::Matrix33>)
                {
                    ss << "Matrix33{"
                        << "{" << v[0][0] << ", " << v[0][1] << ", " << v[0][2] << "}, "
                        << "{" << v[1][0] << ", " << v[1][1] << ", " << v[1][2] << "}, "
                        << "{" << v[2][0] << ", " << v[2][1] << ", " << v[2][2] << "}}";
                }
                else if constexpr (std::is_same_v<ValueType, mx::Matrix44>)
                {
                    ss << "Matrix44{"
                        << "{" << v[0][0] << ", " << v[0][1] << ", " << v[0][2] << ", " << v[0][3] << "}, "
                        << "{" << v[1][0] << ", " << v[1][1] << ", " << v[1][2] << ", " << v[1][3] << "}, "
                        << "{" << v[2][0] << ", " << v[2][1] << ", " << v[2][2] << ", " << v[2][3] << "}, "
                        << "{" << v[3][0] << ", " << v[3][1] << ", " << v[3][2] << ", " << v[3][3] << "}}";
                }
                else
                {
                    ss << std::boolalpha << v;
                }
                
                return ss.str();
            },
            value_
        );
    }
}
