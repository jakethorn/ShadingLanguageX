//
// Created by jaket on 05/07/2026.
//

#include <sstream>

#include "Primitive.h"

#include "common.h"
#include "utils/primitive_utils.h"
#include "runtime/Type.h"

namespace mxslc
{
    Primitive::Primitive(const mx::ValuePtr& value)
    {
#define INIT_IF(type) if constexpr (not std::is_same_v<type, fs::path>) { if (value->isA<type>()) { value_ = value->asA<type>(); return; } }
        FOR_EACH_PRIMITIVE_TYPE(INIT_IF, )
#undef INIT_IF
    }

    Primitive::Primitive(const TypePtr& type)
    {
#define INIT_IF(t) if (type->is<t>()) { value_ = t{}; return; }
        FOR_EACH_PRIMITIVE_TYPE(INIT_IF, )
#undef INIT_IF
    }

    TypePtr Primitive::type() const
    {
        return visit([](const auto& v) -> TypePtr {
            IF_VISITED_TYPE_IS(std::monostate)
                return Type::Void;
            return Type::of<VISITED_TYPE>();
        });
    }

    string Primitive::type_name() const
    {
        return type()->name();
    }

    bool Primitive::is_a(const TypePtr& type) const
    {
        return visit([&type](const auto& v) -> bool {
            IF_VISITED_TYPE_IS(std::monostate)
                return false;
            return type->is<VISITED_TYPE>();
        });
    }

    bool Primitive::is_vector_type() const
    {
        return type()->is_vector();
    }

    bool Primitive::is_null() const
    {
        return std::holds_alternative<std::monostate>(value_);
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

    Primitive Primitive::operator%(const Primitive& other) const
    {
        return primitive_utils::modulo(*this, other);
    }

    Primitive& Primitive::operator+=(const Primitive& other)
    {
        *this = *this + other;
        return *this;
    }

    Primitive& Primitive::operator-=(const Primitive& other)
    {
        *this = *this - other;
        return *this;
    }

    Primitive& Primitive::operator*=(const Primitive& other)
    {
        *this = *this * other;
        return *this;
    }

    Primitive& Primitive::operator/=(const Primitive& other)
    {
        *this = *this / other;
        return *this;
    }

    Primitive& Primitive::operator%=(const Primitive& other)
    {
        *this = *this % other;
        return *this;
    }

    Primitive Primitive::operator&(const Primitive& other) const
    {
        return primitive_utils::logical_and(*this, other);
    }

    Primitive Primitive::operator|(const Primitive& other) const
    {
        return primitive_utils::logical_or(*this, other);
    }

    Primitive Primitive::operator^(const Primitive& other) const
    {
        if (is_a<bool>() and other.is_a<bool>())
            return primitive_utils::logical_xor(*this, other);
        else
            return primitive_utils::power(*this, other);
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

    Primitive::operator bool() const
    {
        return cast<bool>();
    }

    string Primitive::to_string() const
    {
        return visit([](const auto& v) {
            std::stringstream ss;

            IF_VISITED_TYPE_IS(std::monostate)
            {
                ss << "null";
            }
            else IF_VISITED_TYPE_IS(mx::Vector2)
            {
                ss << "vec2{" << v[0] << ", " << v[1] << "}";
            }
            else IF_VISITED_TYPE_IS(mx::Vector3)
            {
                ss << "vec3{" << v[0] << ", " << v[1] << ", " << v[2] << "}";
            }
            else IF_VISITED_TYPE_IS(mx::Vector4)
            {
                ss << "vec4{" << v[0] << ", " << v[1] << ", " <<  v[2] << ", " << v[3] << "}";
            }
            else IF_VISITED_TYPE_IS(mx::Color3)
            {
                ss << "color3{" << v[0] << ", " << v[1] << ", " << v[2] << "}";
            }
            else IF_VISITED_TYPE_IS(mx::Color4)
            {
                ss << "Color4{" << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << "}";
            }
            else IF_VISITED_TYPE_IS(mx::Matrix33)
            {
                ss << "Matrix33{"
                    << "{" << v[0][0] << ", " << v[0][1] << ", " << v[0][2] << "}, "
                    << "{" << v[1][0] << ", " << v[1][1] << ", " << v[1][2] << "}, "
                    << "{" << v[2][0] << ", " << v[2][1] << ", " << v[2][2] << "}}";
            }
            else IF_VISITED_TYPE_IS(mx::Matrix44)
            {
                ss << "Matrix44{"
                    << "{" << v[0][0] << ", " << v[0][1] << ", " << v[0][2] << ", " << v[0][3] << "}, "
                    << "{" << v[1][0] << ", " << v[1][1] << ", " << v[1][2] << ", " << v[1][3] << "}, "
                    << "{" << v[2][0] << ", " << v[2][1] << ", " << v[2][2] << ", " << v[2][3] << "}, "
                    << "{" << v[3][0] << ", " << v[3][1] << ", " << v[3][2] << ", " << v[3][3] << "}}";
            }
            else IF_VISITED_TYPE_IS(string)
            {
                ss << "\"" << v << "\"";
            }
            else
            {
                ss << std::boolalpha << v;
            }

            return ss.str();
        });
    }
}
