//
// Created by jaket on 28/05/2026.
//

#include "mtlx/node_evaluators/convert.h"
#include "runtime/Type.h"
#include "runtime/Variable.h"
#include "values/BasicValue.h"

#define CONVERT_SINGLE_TO_VECTOR(fromtype, totype) \
    if (type->is<totype>()) \
        return Variable::create(totype{static_cast<float>(in->get<fromtype>())});

#define CONVERT_VECTOR_TO_VECTOR(fromtype, totype) \
    if (in->is<fromtype>()) \
    { \
        if (type->is<totype>()) \
        { \
            vector<float> data(totype::numElements(), 0.0f); \
            for (size_t i = 0; i < std::min(fromtype::numElements(), totype::numElements()); ++i) \
            { \
                data[i] = in->get<fromtype>()[i]; \
            } \
            return Variable::create(totype{data}); \
        } \
    }

namespace mxslc
{
VarPtr evaluate_convert(const TypePtr& type, const vector<BasicValuePtr>& values)
{
    const BasicValuePtr& in = values[0];
    if (in->is<bool>())
    {
        if (type->is<int>())
        {
            int out = in->get<bool>();
            return Variable::create(out);
        }

        if (type->is<float>())
        {
            float out = in->get<bool>();
            return Variable::create(out);
        }

        CONVERT_SINGLE_TO_VECTOR(bool, mx::Vector2)
        CONVERT_SINGLE_TO_VECTOR(bool, mx::Vector3)
        CONVERT_SINGLE_TO_VECTOR(bool, mx::Vector4)
        CONVERT_SINGLE_TO_VECTOR(bool, mx::Color3)
        CONVERT_SINGLE_TO_VECTOR(bool, mx::Color4)
    }

    if (in->is<int>())
    {
        if (type->is<bool>())
        {
            bool out = in->get<int>();
            return Variable::create(out);
        }

        if (type->is<float>())
        {
            float out = static_cast<float>(in->get<int>());
            return Variable::create(out);
        }

        CONVERT_SINGLE_TO_VECTOR(int, mx::Vector2)
        CONVERT_SINGLE_TO_VECTOR(int, mx::Vector3)
        CONVERT_SINGLE_TO_VECTOR(int, mx::Vector4)
        CONVERT_SINGLE_TO_VECTOR(int, mx::Color3)
        CONVERT_SINGLE_TO_VECTOR(int, mx::Color4)
    }

    if (in->is<float>())
    {
        CONVERT_SINGLE_TO_VECTOR(float, mx::Vector2)
        CONVERT_SINGLE_TO_VECTOR(float, mx::Vector3)
        CONVERT_SINGLE_TO_VECTOR(float, mx::Vector4)
        CONVERT_SINGLE_TO_VECTOR(float, mx::Color3)
        CONVERT_SINGLE_TO_VECTOR(float, mx::Color4)
    }

    CONVERT_VECTOR_TO_VECTOR(mx::Vector2, mx::Vector3)
    CONVERT_VECTOR_TO_VECTOR(mx::Vector2, mx::Vector4)
    CONVERT_VECTOR_TO_VECTOR(mx::Vector2, mx::Color3)
    CONVERT_VECTOR_TO_VECTOR(mx::Vector2, mx::Color4)

    CONVERT_VECTOR_TO_VECTOR(mx::Vector3, mx::Vector2)
    CONVERT_VECTOR_TO_VECTOR(mx::Vector3, mx::Vector4)
    CONVERT_VECTOR_TO_VECTOR(mx::Vector3, mx::Color3)
    CONVERT_VECTOR_TO_VECTOR(mx::Vector3, mx::Color4)

    CONVERT_VECTOR_TO_VECTOR(mx::Vector4, mx::Vector2)
    CONVERT_VECTOR_TO_VECTOR(mx::Vector4, mx::Vector3)
    CONVERT_VECTOR_TO_VECTOR(mx::Vector4, mx::Color3)
    CONVERT_VECTOR_TO_VECTOR(mx::Vector4, mx::Color4)

    CONVERT_VECTOR_TO_VECTOR(mx::Color3, mx::Vector2)
    CONVERT_VECTOR_TO_VECTOR(mx::Color3, mx::Vector3)
    CONVERT_VECTOR_TO_VECTOR(mx::Color3, mx::Vector4)
    CONVERT_VECTOR_TO_VECTOR(mx::Color3, mx::Color4)

    CONVERT_VECTOR_TO_VECTOR(mx::Color4, mx::Vector2)
    CONVERT_VECTOR_TO_VECTOR(mx::Color4, mx::Vector3)
    CONVERT_VECTOR_TO_VECTOR(mx::Color4, mx::Vector4)
    CONVERT_VECTOR_TO_VECTOR(mx::Color4, mx::Color3)

    return nullptr;
}

#undef CONVERT_SINGLE_TO_VECTOR
#undef CONVERT_VECTOR_TO_VECTOR
}