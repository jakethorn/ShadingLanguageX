//
// Created by jaket on 05/07/2026.
//

#ifndef MXSLC_PRIMITIVE_H
#define MXSLC_PRIMITIVE_H

#include <variant>

#include <MaterialXCore/Types.h>

#include "common.h"
#include "runtime/utils/type_utils.h"
#include "errors/CompileError.h"
#include "utils/Stringable.h"

namespace mxslc
{
    class Primitive : public Stringable
    {
    public:
#define CTOR(T) Primitive(T value) : value_{value} { }
        CTOR(bool)
        CTOR(int)
        CTOR(float)
        CTOR(string)
        CTOR(fs::path)
        CTOR(mx::Vector2)
        CTOR(mx::Vector3)
        CTOR(mx::Vector4)
        CTOR(mx::Color3)
        CTOR(mx::Color4)
        CTOR(mx::Matrix33)
        CTOR(mx::Matrix44)
#undef CTOR

        Primitive(const std::array<float, 2> value) : value_{mx::Vector2{value}} { }
        Primitive(const std::array<float, 3> value) : value_{mx::Vector3{value}} { }
        Primitive(const std::array<float, 4> value) : value_{mx::Vector4{value}} { }

        const TypePtr& type() const;
        const string& type_name() const;

        template<typename T>
        bool is_a() const
        {
            assert_type<T>();
            return std::holds_alternative<T>(value_);
        }

        template<typename T>
        T as() const
        {
            assert_type<T>();

            if (is_a<T>())
                return std::get<T>(value_);
            throw CompileError{"Invalid access of primitive of type '" + type_name() + "' as a '" + type_utils::name_of<T>() + "'"};
        }

        template<typename T>
        T cast() const
        {
            assert_type<T>();

            if (is_a<T>())
                return as<T>();

            return std::visit(
                [](const auto& v) -> T {
                    using VariantType = std::decay_t<decltype(v)>;
                    if constexpr (std::is_convertible_v<VariantType, T>)
                        return static_cast<T>(v);
                    else if constexpr (std::is_constructible_v<T, VariantType>)
                        return T(v);
                    else
                        throw CompileError{"Invalid cast of primitive of type '" + type_utils::name_of<VariantType>() + "' to '" + type_utils::name_of<T>() + "'"};
                },
                value_
            );
        }

        template<typename T>
        bool is_castable_to() const
        {
            assert_type<T>();

            if (is_a<T>())
                return true;

            return std::visit(
                [](const auto& v) -> bool {
                    using VariantType = std::decay_t<decltype(v)>;
                    return std::is_convertible_v<VariantType, T> or std::is_constructible_v<T, VariantType>;
                },
                value_
            );
        }

        Primitive convert(const TypePtr& type) const;

        Primitive operator+(const Primitive& other) const;
        Primitive operator-(const Primitive& other) const;
        Primitive operator*(const Primitive& other) const;
        Primitive operator/(const Primitive& other) const;

        Primitive operator and(const Primitive& other) const;
        Primitive operator or(const Primitive& other) const;
        Primitive operator xor(const Primitive& other) const;

        Primitive operator==(const Primitive& other) const;
        Primitive operator!=(const Primitive& other) const;
        Primitive operator>(const Primitive& other) const;
        Primitive operator<(const Primitive& other) const;
        Primitive operator>=(const Primitive& other) const;
        Primitive operator<=(const Primitive& other) const;

        Primitive operator+() const;
        Primitive operator-() const;
        Primitive operator!() const;

        Primitive operator[](size_t index) const;
        Primitive operator[](const Primitive& index) const;

        string to_string() const override;

    private:
        template<typename T>
        constexpr void assert_type() const
        {
            constexpr bool primitive_type_is_valid =
                std::is_same_v<T, bool> or
                std::is_same_v<T, int> or
                std::is_same_v<T, float> or
                std::is_same_v<T, string> or
                std::is_same_v<T, fs::path> or
                std::is_same_v<T, mx::Vector2> or
                std::is_same_v<T, mx::Vector3> or
                std::is_same_v<T, mx::Vector4> or
                std::is_same_v<T, mx::Color3> or
                std::is_same_v<T, mx::Color4> or
                std::is_same_v<T, mx::Matrix33> or
                std::is_same_v<T, mx::Matrix44>;

            static_assert(primitive_type_is_valid);
        }

        std::variant<
            bool,
            int,
            float,
            string,
            fs::path,
            mx::Vector2,
            mx::Vector3,
            mx::Vector4,
            mx::Color3,
            mx::Color4,
            mx::Matrix33,
            mx::Matrix44
        > value_;
    };
}

#endif //MXSLC_PRIMITIVE_H
