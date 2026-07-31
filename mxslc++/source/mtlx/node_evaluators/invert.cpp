//
// Created by jaket on 31/07/2026.
//

#include "invert.h"

#include "CompileError.h"
#include "runtime/Type.h"
#include "runtime/Variable.h"
#include "values/BasicValue.h"

namespace
{
    float as_float(const BasicValuePtr& value)
    {
        if (value->is<float>())
            return value->get<float>();
        if (value->is<int>())
            return static_cast<float>(value->get<int>());
        throw CompileError{"Cannot convert value to float"s};
    }
}

// MaterialX invert computes: out = amount - in
VarPtr evaluate_invert(const TypePtr&, const vector<BasicValuePtr>& values)
{
    const auto& in = values[0];
    const auto& amount = values[1];

    // scalar: amount - in
    if (in->is<int, float>() and amount->is<int, float>())
        return Variable::create(as_float(amount) - as_float(in));

#define EVAL_INVERT_VEC(vectype) \
    if (in->is<vectype>()) \
    { \
        const vectype v = in->get<vectype>(); \
        if (amount->is<int, float>()) \
        { \
            const float a = as_float(amount); \
            vectype out; \
            for (size_t i = 0; i < vectype::numElements(); ++i) \
                out[i] = a - v[i]; \
            return Variable::create(out); \
        } \
        if (amount->is<vectype>()) \
        { \
            const vectype a = amount->get<vectype>(); \
            vectype out; \
            for (size_t i = 0; i < vectype::numElements(); ++i) \
                out[i] = a[i] - v[i]; \
            return Variable::create(out); \
        } \
    }

    EVAL_INVERT_VEC(mx::Vector2)
    EVAL_INVERT_VEC(mx::Vector3)
    EVAL_INVERT_VEC(mx::Vector4)
    EVAL_INVERT_VEC(mx::Color3)
    EVAL_INVERT_VEC(mx::Color4)

#undef EVAL_INVERT_VEC

    return nullptr;
}
