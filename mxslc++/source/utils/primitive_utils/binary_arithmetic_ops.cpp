//
// Created by jaket on 11/07/2026.
//

#include "utils/primitive_utils.h"
#include "runtime/Type.h"

#define ARITHMETIC_OP_IF(type, op) \
    if ((in1.is_a<type>() and in2.is_castable_to<type>()) or (in2.is_a<type>() and in1.is_castable_to<type>())) \
        return Primitive{in1.cast<type>() op in2.cast<type>()}.cast<type>();

#define TO_STRING_HELPER(x) #x
#define TO_STRING(x) TO_STRING_HELPER(x)

namespace mxslc::primitive_utils
{
    Primitive add(const Primitive& in1, const Primitive& in2)
    {
#define OP +
        ARITHMETIC_OP_IF(float, OP)
        ARITHMETIC_OP_IF(int, OP)
        ARITHMETIC_OP_IF(bool, OP)
        ARITHMETIC_OP_IF(mx::Vector2, OP)
        ARITHMETIC_OP_IF(mx::Vector3, OP)
        ARITHMETIC_OP_IF(mx::Vector4, OP)
        ARITHMETIC_OP_IF(mx::Color3, OP)
        ARITHMETIC_OP_IF(mx::Color4, OP)
        ARITHMETIC_OP_IF(mx::Matrix33, OP)
        ARITHMETIC_OP_IF(mx::Matrix44, OP)
        ARITHMETIC_OP_IF(string, OP)

        throw CompileError{"Binary operator " TO_STRING(OP) " not supported for values of type '" + in1.type_name() + "' and '" + in2.type_name() + "'"};
#undef OP
    }

    Primitive subtract(const Primitive& in1, const Primitive& in2)
    {
#define OP -
        ARITHMETIC_OP_IF(float, -)
        ARITHMETIC_OP_IF(int, -)
        ARITHMETIC_OP_IF(bool, -)
        ARITHMETIC_OP_IF(mx::Vector2, -)
        ARITHMETIC_OP_IF(mx::Vector3, -)
        ARITHMETIC_OP_IF(mx::Vector4, -)
        ARITHMETIC_OP_IF(mx::Color3, -)
        ARITHMETIC_OP_IF(mx::Color4, -)
        ARITHMETIC_OP_IF(mx::Matrix33, -)
        ARITHMETIC_OP_IF(mx::Matrix44, -)

        throw CompileError{"Binary operator " TO_STRING(OP) " not supported for values of type '" + in1.type_name() + "' and '" + in2.type_name() + "'"};
#undef OP
    }

    Primitive multiply(const Primitive& in1, const Primitive& in2)
    {
#define OP *
        ARITHMETIC_OP_IF(float, OP)
        ARITHMETIC_OP_IF(int, OP)
        ARITHMETIC_OP_IF(bool, OP)
        ARITHMETIC_OP_IF(mx::Vector2, OP)
        ARITHMETIC_OP_IF(mx::Vector3, OP)
        ARITHMETIC_OP_IF(mx::Vector4, OP)
        ARITHMETIC_OP_IF(mx::Color3, OP)
        ARITHMETIC_OP_IF(mx::Color4, OP)
        ARITHMETIC_OP_IF(mx::Matrix33, OP)
        ARITHMETIC_OP_IF(mx::Matrix44, OP)

        throw CompileError{"Binary operator " TO_STRING(OP) " not supported for values of type '" + in1.type_name() + "' and '" + in2.type_name() + "'"};
#undef OP
    }

    Primitive divide(const Primitive& in1, const Primitive& in2)
    {
#define OP /
        ARITHMETIC_OP_IF(float, OP)
        ARITHMETIC_OP_IF(int, OP)
        ARITHMETIC_OP_IF(bool, OP)
        ARITHMETIC_OP_IF(mx::Vector2, OP)
        ARITHMETIC_OP_IF(mx::Vector3, OP)
        ARITHMETIC_OP_IF(mx::Vector4, OP)
        ARITHMETIC_OP_IF(mx::Color3, OP)
        ARITHMETIC_OP_IF(mx::Color4, OP)
        ARITHMETIC_OP_IF(mx::Matrix33, OP)
        ARITHMETIC_OP_IF(mx::Matrix44, OP)
        ARITHMETIC_OP_IF(fs::path, OP)

        if (in1.is_a<string>() and in2.is_a<string>())
            return Primitive{in1.cast<fs::path>() / in2.cast<fs::path>()}.cast<string>();

        throw CompileError{"Binary operator " TO_STRING(OP) " not supported for values of type '" + in1.type_name() + "' and '" + in2.type_name() + "'"};
#undef OP
    }

    namespace
    {
        int imod(const int lhs, const int rhs)
        {
            return lhs % rhs;
        }

        template<typename T>
        T vmod(const T& lhs, const T& rhs)
        {
            vector<float> result;
            result.reserve(T::numElements());
            for (size_t i = 0; i < T::numElements(); ++i)
                result.push_back(std::fmod(lhs[i], rhs[i]));
            return T{result};
        }
    }

    Primitive modulo(const Primitive& in1, const Primitive& in2)
    {
#define MODULO_IF(type, func) \
if ((in1.is_a<type>() and in2.is_castable_to<type>()) or (in2.is_a<type>() and in1.is_castable_to<type>())) \
return func(in1.cast<type>(), in2.cast<type>());

        MODULO_IF(float, std::fmod)
        MODULO_IF(int, imod)
        MODULO_IF(mx::Vector2, vmod)
        MODULO_IF(mx::Vector3, vmod)
        MODULO_IF(mx::Vector4, vmod)
        MODULO_IF(mx::Color3, vmod)
        MODULO_IF(mx::Color4, vmod)

        throw CompileError{"Binary operator % not supported for values of type '" + in1.type_name() + "' and '" + in2.type_name() + "'"};
#undef MODULO_IF
    }

    namespace
    {
        int ipow(const int lhs, const int rhs)
        {
            if (rhs < 0)
                return 0;

            int result = 1;
            for (int i = 0; i < rhs; ++i)
                result *= lhs;
            return result;
        }

        template<typename T>
        T vpow(const T& lhs, const T& rhs)
        {
            vector<float> result;
            result.reserve(T::numElements());
            for (size_t i = 0; i < T::numElements(); ++i)
                result.push_back(std::pow(lhs[i], rhs[i]));
            return T{result};
        }
    }

    Primitive power(const Primitive& in1, const Primitive& in2)
    {
#define POWER_IF(type, func) \
    if ((in1.is_a<type>() and in2.is_castable_to<type>()) or (in2.is_a<type>() and in1.is_castable_to<type>())) \
        return func(in1.cast<type>(), in2.cast<type>());

        POWER_IF(float, std::pow)
        POWER_IF(int, ipow)
        POWER_IF(mx::Vector2, vpow)
        POWER_IF(mx::Vector3, vpow)
        POWER_IF(mx::Vector4, vpow)
        POWER_IF(mx::Color3, vpow)
        POWER_IF(mx::Color4, vpow)

        throw CompileError{"Binary operator ^ not supported for values of type '" + in1.type_name() + "' and '" + in2.type_name() + "'"};
#undef POWER_IF
    }
}

#undef TO_STRING
#undef TO_STRING_HELPER
#undef ARITHMETIC_OP_IF
