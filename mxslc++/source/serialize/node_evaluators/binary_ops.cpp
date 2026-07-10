//
// Created by jaket on 28/05/2026.
//

#include "serialize/node_evaluators/binary_ops.h"
#include "serialize/values/BasicValue.h"
#include "runtime/Variable.h"
#include "utils/mtlx_type_binary_ops.h"

#define BINARY_OP(ltype, op, rtype) \
    if (values[0]->is<ltype>() && values[1]->is<rtype>()) \
        return Variable::create(values[0]->get<ltype>() op values[1]->get<rtype>());

namespace mxslc::serialize
{
    VarPtr evaluate_add(const TypePtr&, const vector<BasicValuePtr>& values)
    {
        BINARY_OP(int, +, int)
        BINARY_OP(float, +, float)
        BINARY_OP(mx::Vector2, +, mx::Vector2)
        BINARY_OP(mx::Vector3, +, mx::Vector3)
        BINARY_OP(mx::Vector4, +, mx::Vector4)
        BINARY_OP(mx::Color3, +, mx::Color3)
        BINARY_OP(mx::Color4, +, mx::Color4)
        BINARY_OP(mx::Matrix33, +, mx::Matrix33)
        BINARY_OP(mx::Matrix44, +, mx::Matrix44)
        BINARY_OP(int, +, float)
        BINARY_OP(mx::Vector2, +, float)
        BINARY_OP(mx::Vector3, +, float)
        BINARY_OP(mx::Vector4, +, float)
        BINARY_OP(mx::Color3, +, float)
        BINARY_OP(mx::Color4, +, float)
        BINARY_OP(mx::Matrix33, +, float)
        BINARY_OP(mx::Matrix44, +, float)
        return nullptr;
    }

    VarPtr evaluate_subtract(const TypePtr&, const vector<BasicValuePtr>& values)
    {
        BINARY_OP(int, -, int)
        BINARY_OP(float, -, float)
        BINARY_OP(mx::Vector2, -, mx::Vector2)
        BINARY_OP(mx::Vector3, -, mx::Vector3)
        BINARY_OP(mx::Vector4, -, mx::Vector4)
        BINARY_OP(mx::Color3, -, mx::Color3)
        BINARY_OP(mx::Color4, -, mx::Color4)
        BINARY_OP(mx::Matrix33, -, mx::Matrix33)
        BINARY_OP(mx::Matrix44, -, mx::Matrix44)
        BINARY_OP(int, -, float)
        BINARY_OP(mx::Vector2, -, float)
        BINARY_OP(mx::Vector3, -, float)
        BINARY_OP(mx::Vector4, -, float)
        BINARY_OP(mx::Color3, -, float)
        BINARY_OP(mx::Color4, -, float)
        BINARY_OP(mx::Matrix33, -, float)
        BINARY_OP(mx::Matrix44, -, float)
        return nullptr;
    }

    VarPtr evaluate_multiply(const TypePtr&, const vector<BasicValuePtr>& values)
    {
        BINARY_OP(float, *, float)
        BINARY_OP(mx::Vector2, *, mx::Vector2)
        BINARY_OP(mx::Vector3, *, mx::Vector3)
        BINARY_OP(mx::Vector4, *, mx::Vector4)
        BINARY_OP(mx::Color3, *, mx::Color3)
        BINARY_OP(mx::Color4, *, mx::Color4)
        BINARY_OP(mx::Matrix33, *, mx::Matrix33)
        BINARY_OP(mx::Matrix44, *, mx::Matrix44)
        BINARY_OP(mx::Vector2, *, float)
        BINARY_OP(mx::Vector3, *, float)
        BINARY_OP(mx::Vector4, *, float)
        BINARY_OP(mx::Color3, *, float)
        BINARY_OP(mx::Color4, *, float)
        BINARY_OP(mx::Matrix33, *, float)
        BINARY_OP(mx::Matrix44, *, float)
        return nullptr;
    }

    VarPtr evaluate_divide(const TypePtr&, const vector<BasicValuePtr>& values)
    {
        BINARY_OP(float, /, float)
        BINARY_OP(mx::Vector2, /, mx::Vector2)
        BINARY_OP(mx::Vector3, /, mx::Vector3)
        BINARY_OP(mx::Vector4, /, mx::Vector4)
        BINARY_OP(mx::Color3, /, mx::Color3)
        BINARY_OP(mx::Color4, /, mx::Color4)
        BINARY_OP(mx::Matrix33, /, mx::Matrix33)
        BINARY_OP(mx::Matrix44, /, mx::Matrix44)
        BINARY_OP(mx::Vector2, /, float)
        BINARY_OP(mx::Vector3, /, float)
        BINARY_OP(mx::Vector4, /, float)
        BINARY_OP(mx::Color3, /, float)
        BINARY_OP(mx::Color4, /, float)
        BINARY_OP(mx::Matrix33, /, float)
        BINARY_OP(mx::Matrix44, /, float)
        return nullptr;
    }
}

#undef BINARY_OP