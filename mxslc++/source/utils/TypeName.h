//
// Created by jaket on 01/06/2026.
//

#ifndef MXSLC_TYPENAME_H
#define MXSLC_TYPENAME_H

#include "common.h"

class TypeName
{
public:
    inline static string Bool = "boolean";
    inline static string Int = "integer";
    inline static string Float = "float";
    inline static string String = "string";
    inline static string Filename = "filename";
    inline static string Vec2 = "vector2";
    inline static string Vec3 = "vector3";
    inline static string Vec4 = "vector4";
    inline static string Color3 = "color3";
    inline static string Color4 = "color4";
    inline static string Mat3 = "matrix33";
    inline static string Mat4 = "matrix44";
    inline static string Void = "void";
    inline static string Auto = "auto";

    template<typename T>
    static const string& get()
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

#endif //MXSLC_TYPENAME_H
