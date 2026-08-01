//
// Created by jaket on 01/06/2026.
//

#ifndef MXSLC_TYPENAME_H
#define MXSLC_TYPENAME_H

#include <MaterialXCore/Types.h>

#include "common.h"
#include "errors/CompileError.h"

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
#define NAME_OF(type, name) if constexpr (std::is_same_v<T, type>) { return name; }
            NAME_OF(bool, Bool)
            NAME_OF(int, Int)
            NAME_OF(float, Float)
            NAME_OF(string, String)
            NAME_OF(fs::path, Filename)
            NAME_OF(mx::Vector2, Vec2)
            NAME_OF(mx::Vector3, Vec3)
            NAME_OF(mx::Vector4, Vec4)
            NAME_OF(mx::Color3, Color3)
            NAME_OF(mx::Color4, Color4)
            NAME_OF(mx::Matrix33, Mat3)
            NAME_OF(mx::Matrix44, Mat4)
            NAME_OF(void, Void)
#undef NAME_OF

            throw CompileError{"Unknown type"};
        }

        static bool is_primitive(const string& type_name)
        {
            return  type_name == Bool or type_name == Int or type_name == Float or type_name == String or type_name == Filename or
                    type_name == Vec2 or type_name == Vec3 or type_name == Vec4 or
                    type_name == Color3 or type_name == Color4 or
                    type_name == Mat3 or type_name == Mat4;
        }

        static bool is_single(const string& type_name)
        {
            return type_name == Bool or type_name == Int or type_name == Float;
        }

        static bool is_vector(const string& type_name)
        {
            return type_name == Vec2 or type_name == Vec3 or type_name == Vec4 or type_name == Color3 or type_name == Color4;
        }

        static bool is_matrix(const string& type_name)
        {
            return type_name == Mat3 or type_name == Mat4;
        }
    };
}

#endif //MXSLC_TYPENAME_H
