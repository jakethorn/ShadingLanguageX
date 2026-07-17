//
// Created by jaket on 05/07/2026.
//

#ifndef MXSLC_PRIMITIVE_H
#define MXSLC_PRIMITIVE_H

#include <variant>

#include <MaterialXCore/Types.h>
#include <MaterialXCore/Value.h>

#include "common.h"
#include "runtime/utils/type_utils.h"
#include "errors/CompileError.h"
#include "utils/Stringable.h"

#define COMMA ,
#define FOR_EACH_PRIMITIVE_TYPE(DO, sep) \
    DO(bool) sep \
    DO(int) sep \
    DO(float) sep \
    DO(string) sep \
    DO(fs::path) sep \
    DO(mx::Vector2) sep \
    DO(mx::Vector3) sep \
    DO(mx::Vector4) sep \
    DO(mx::Color3) sep \
    DO(mx::Color4) sep \
    DO(mx::Matrix33) sep \
    DO(mx::Matrix44)

#define VISITED_TYPE std::decay_t<decltype(v)>
#define IF_VISITED_TYPE_IS(type) if constexpr (std::is_same_v<VISITED_TYPE, type>)

namespace mxslc
{
    class Primitive : public Stringable
    {
    public:
        Primitive() : value_{std::monostate{}} { }

#define CTOR(type) Primitive(type value) : value_{value} { }
        FOR_EACH_PRIMITIVE_TYPE(CTOR, )
#undef CTOR

        Primitive(const size_t value) : value_{static_cast<int>(value)} { }
        Primitive(const char* value) : value_{string{value}} { }
        Primitive(const std::array<float, 2> value) : value_{mx::Vector2{value}} { }
        Primitive(const std::array<float, 3> value) : value_{mx::Vector3{value}} { }
        Primitive(const std::array<float, 4> value) : value_{mx::Vector4{value}} { }

        explicit Primitive(const mx::ValuePtr& value);
        explicit Primitive(const TypePtr& type);

        TypePtr type() const;
        string type_name() const;

        bool is_a(const TypePtr& type) const;
        bool is_vector_type() const;
        bool is_null() const;

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
        std::array<Primitive, 2> separate2() const;
        std::array<Primitive, 3> separate3() const;
        std::array<Primitive, 4> separate4() const;
        vector<Primitive> separate() const;

        Primitive operator+(const Primitive& other) const;
        Primitive operator-(const Primitive& other) const;
        Primitive operator*(const Primitive& other) const;
        Primitive operator/(const Primitive& other) const;

        Primitive& operator+=(const Primitive& other);
        Primitive& operator-=(const Primitive& other);
        Primitive& operator*=(const Primitive& other);
        Primitive& operator/=(const Primitive& other);

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

        explicit operator bool() const;

        template<typename Visitor>
        decltype(auto) visit(Visitor&& vis)
        {
            return std::visit(std::forward<Visitor>(vis), value_);
        }

        template<typename Visitor>
        decltype(auto) visit(Visitor&& vis) const
        {
            return std::visit(std::forward<Visitor>(vis), value_);
        }

        string to_string() const override;

    private:
        template<typename T>
        constexpr void assert_type() const
        {
#define IS_TYPE(type) std::is_same_v<T, type>
            static_assert(FOR_EACH_PRIMITIVE_TYPE(IS_TYPE, or));
#undef IS_TYPE
        }

        std::variant<
#define DECL(type) type
            std::monostate,
            FOR_EACH_PRIMITIVE_TYPE(DECL, COMMA)
#undef DECL
        > value_;
    };
}

#endif //MXSLC_PRIMITIVE_H
