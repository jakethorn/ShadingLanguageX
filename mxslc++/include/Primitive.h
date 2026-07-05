//
// Created by jaket on 05/07/2026.
//

#ifndef MXSLC_PRIMITIVE_H
#define MXSLC_PRIMITIVE_H

#include "CompileError.h"
#include "primitive_t.h"
#include "runtime/Type.h"

#define CTOR(T) Primitive(T value) : value_{value} { }

namespace mxslc
{
    class Primitive
    {
    public:
        CTOR(bool)
        CTOR(int)
        CTOR(float)
        CTOR(std::string)
        CTOR(std::filesystem::path)
        CTOR(MaterialX::Vector2)
        CTOR(MaterialX::Vector3)
        CTOR(MaterialX::Vector4)
        CTOR(MaterialX::Color3)
        CTOR(MaterialX::Color4)
        CTOR(MaterialX::Matrix33)
        CTOR(MaterialX::Matrix44)

        template<typename T>
        bool is_a() const
        {
            return std::holds_alternative<T>(value_);
        }

        template<typename T>
        T as() const
        {
            if (is_a<T>())
                return std::get<T>(value_);
            throw CompileError{"'" + str() + "' is not a " + "type_utils::name_of<T>()"};
        }

        std::string str() const;

    private:
        primitive_t value_;
    };
}

#undef CTOR

#endif //MXSLC_PRIMITIVE_H
