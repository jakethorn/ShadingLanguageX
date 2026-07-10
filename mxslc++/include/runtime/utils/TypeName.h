//
// Created by jaket on 01/06/2026.
//

#ifndef MXSLC_TYPENAME_H
#define MXSLC_TYPENAME_H

#include "common.h"
#include "primitive_t.h"

namespace mxslc::runtime_utils
{
    class TypeName
    {
    public:
        inline static const string Bool = "boolean";
        inline static const string Int = "integer";
        inline static const string Float = "float";
        inline static const string String = "string";
        inline static const string Filename = "filename";
        inline static const string Vec2 = "vector2";
        inline static const string Vec3 = "vector3";
        inline static const string Vec4 = "vector4";
        inline static const string Color3 = "color3";
        inline static const string Color4 = "color4";
        inline static const string Mat3 = "matrix33";
        inline static const string Mat4 = "matrix44";
        inline static const string Void = "void";
        inline static const string Auto = "auto";

        template<typename T>
        static const string& of()
        {
            if constexpr (std::is_same_v<T, bool>) return Bool;
            if constexpr (std::is_same_v<T, int>) return Int;
            if constexpr (std::is_same_v<T, float>) return Float;
            if constexpr (std::is_same_v<T, string>) return String;
            if constexpr (std::is_same_v<T, mx::Vector2>) return Vec2;
            if constexpr (std::is_same_v<T, mx::Vector3>) return Vec3;
            if constexpr (std::is_same_v<T, mx::Vector4>) return Vec4;
            if constexpr (std::is_same_v<T, mx::Color3>) return Color3;
            if constexpr (std::is_same_v<T, mx::Color4>) return Color4;
            if constexpr (std::is_same_v<T, mx::Matrix33>) return Mat3;
            if constexpr (std::is_same_v<T, mx::Matrix44>) return Mat4;
            if constexpr (std::is_same_v<T, void>) return Void;
            throw std::runtime_error("Unknown type");
        }

        static const string& of(const primitive_t& value)
        {
    #define type_of(t, p) if (std::holds_alternative<t>(value)) return p;
            type_of(bool, Bool);
            type_of(int, Int);
            type_of(float, Float);
            type_of(string, String);
            type_of(fs::path, Filename);
            type_of(mx::Vector2, Vec2);
            type_of(mx::Vector3, Vec3);
            type_of(mx::Vector4, Vec4);
            type_of(mx::Color3, Color3);
            type_of(mx::Color4, Color4);
            type_of(mx::Matrix33, Mat3);
            type_of(mx::Matrix44, Mat4);
    #undef type_of
            throw std::runtime_error{"Invalid primitive value"};
        }

        static bool is_primitive(const string& type_name)
        {
            return  type_name == Bool or type_name == Int or type_name == Float or type_name == String or type_name == Filename or
                    type_name == Vec2 or type_name == Vec3 or type_name == Vec4 or
                    type_name == Color3 or type_name == Color4 or
                    type_name == Mat3 or type_name == Mat4;
        }

        static bool is_vector(const string& type_name)
        {
            return type_name == Vec2 or type_name == Vec3 or type_name == Vec4 or type_name == Color3 or type_name == Color4;
        }
    };
}

#endif //MXSLC_TYPENAME_H
