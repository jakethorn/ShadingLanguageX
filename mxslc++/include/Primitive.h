//
// Created by jaket on 05/07/2026.
//

#ifndef MXSLC_PRIMITIVE_H
#define MXSLC_PRIMITIVE_H

#include "common.h"
#include "primitive_t.h"
#include "errors/CompileError.h"

#define CTOR(T) Primitive(T value) : value_{value} { }

namespace mxslc
{
    class Primitive
    {
    public:
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

        string str() const;

    private:
        primitive_t value_;
    };
}

#undef CTOR

#endif //MXSLC_PRIMITIVE_H
