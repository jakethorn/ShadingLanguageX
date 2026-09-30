//
// Created by jaket on 05/07/2026.
//

#include <sstream>

#include "Primitive.h"

#include "common.h"
#include "utils/primitive_utils.h"
#include "utils/string_utils.h"
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

    namespace
    {
        template<typename T>
        string vector_components(const T& v, const size_t count)
        {
            string result;
            for (size_t i = 0; i < count; ++i)
                result += (i > 0 ? ", " : "") + string_utils::format_float(v[i]);
            return result;
        }

        template<typename T>
        string matrix_rows(const T& m, const size_t count, const string& row_type)
        {
            string result;
            for (size_t i = 0; i < count; ++i)
            {
                result += i > 0 ? ", " : "";
                result += row_type + "{";
                for (size_t j = 0; j < count; ++j)
                    result += (j > 0 ? ", " : "") + string_utils::format_float(m[i][j]);
                result += "}";
            }
            return result;
        }
    }

    string Primitive::to_string() const
    {
        return visit([](const auto& v) -> string {
            using string_utils::format_float;
            IF_VISITED_TYPE_IS(std::monostate)
                return "null";
            else IF_VISITED_TYPE_IS(bool)
                return v ? "true" : "false";
            else IF_VISITED_TYPE_IS(int)
                return std::to_string(v);
            else IF_VISITED_TYPE_IS(float)
                return format_float(v);
            else IF_VISITED_TYPE_IS(string)
                return "\"" + v + "\"";
            else IF_VISITED_TYPE_IS(fs::path)
                return "\"" + v.string() + "\"";
            else IF_VISITED_TYPE_IS(mx::Vector2)
                return "vec2{" + vector_components(v, 2) + "}";
            else IF_VISITED_TYPE_IS(mx::Vector3)
                return "vec3{" + vector_components(v, 3) + "}";
            else IF_VISITED_TYPE_IS(mx::Vector4)
                return "vec4{" + vector_components(v, 4) + "}";
            else IF_VISITED_TYPE_IS(mx::Color3)
                return "color3{" + vector_components(v, 3) + "}";
            else IF_VISITED_TYPE_IS(mx::Color4)
                return "color4{" + vector_components(v, 4) + "}";
            else IF_VISITED_TYPE_IS(mx::Matrix33)
                return "creatematrix(" + matrix_rows(v, 3, "vec3") + ")";
            else IF_VISITED_TYPE_IS(mx::Matrix44)
                return "creatematrix(" + matrix_rows(v, 4, "vec4") + ")";
        });
    }
}
