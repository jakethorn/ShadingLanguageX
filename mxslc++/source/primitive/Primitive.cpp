//
// Created by jaket on 05/07/2026.
//

#include "primitive/Primitive.h"
#include "common.h"
#include "primitive/primitive_utils.h"
#include "runtime/Type.h"

namespace mxslc
{
    const TypePtr& Primitive::type() const
    {
#define TYPE_OF(type, name) if (is_a<type>()) { return Type::name; }
        TYPE_OF(bool, Bool)
        TYPE_OF(int, Int)
        TYPE_OF(float, Float)
        TYPE_OF(string, String)
        TYPE_OF(fs::path, Filename)
        TYPE_OF(mx::Vector2, Vec2)
        TYPE_OF(mx::Vector3, Vec3)
        TYPE_OF(mx::Vector4, Vec4)
        TYPE_OF(mx::Color3, Color3)
        TYPE_OF(mx::Color4, Color4)
        TYPE_OF(mx::Matrix33, Mat3)
        TYPE_OF(mx::Matrix44, Mat4)
#undef TYPE_OF

        throw CompileError{"Unknown primitive type"};
    }

    const string& Primitive::type_name() const
    {
        return type()->name();
    }

    Primitive Primitive::convert(const TypePtr& type) const
    {
        return primitive_utils::convert(*this, type);
    }

    Primitive Primitive::operator+(const Primitive& other) const
    {
        return primitive_utils::add(*this, other);
    }

    Primitive Primitive::operator-(const Primitive& other) const
    {
        return primitive_utils::subtract(*this, other);
    }

    Primitive Primitive::operator*(const Primitive& other) const
    {
        return primitive_utils::multiply(*this, other);
    }

    Primitive Primitive::operator/(const Primitive& other) const
    {
        return primitive_utils::divide(*this, other);
    }

    Primitive Primitive::operator and(const Primitive& other) const
    {
        return primitive_utils::logical_and(*this, other);
    }

    Primitive Primitive::operator or(const Primitive& other) const
    {
        return primitive_utils::logical_or(*this, other);
    }

    Primitive Primitive::operator xor(const Primitive& other) const
    {
        return primitive_utils::logical_xor(*this, other);
    }

    Primitive Primitive::operator==(const Primitive& other) const
    {
        return primitive_utils::equal(*this, other);
    }

    Primitive Primitive::operator!=(const Primitive& other) const
    {
        return primitive_utils::not_equal(*this, other);
    }

    Primitive Primitive::operator>(const Primitive& other) const
    {
        return primitive_utils::greater(*this, other);
    }

    Primitive Primitive::operator<(const Primitive& other) const
    {
        return primitive_utils::less(*this, other);
    }

    Primitive Primitive::operator>=(const Primitive& other) const
    {
        return primitive_utils::greater_equal(*this, other);
    }

    Primitive Primitive::operator<=(const Primitive& other) const
    {
        return primitive_utils::less_equal(*this, other);
    }

    Primitive Primitive::operator+() const
    {
        return primitive_utils::positive(*this);
    }

    Primitive Primitive::operator-() const
    {
        return primitive_utils::negative(*this);
    }

    Primitive Primitive::operator!() const
    {
        return primitive_utils::logical_not(*this);
    }

    Primitive Primitive::operator[](const size_t index) const
    {
        return primitive_utils::extract(*this, index);
    }

    Primitive Primitive::operator[](const Primitive& index) const
    {
        return primitive_utils::extract(*this, index);
    }

    string Primitive::to_string() const
    {
        return std::visit(
            [](const auto& v)
            {
                std::stringstream ss;

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
