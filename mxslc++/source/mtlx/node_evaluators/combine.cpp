//
// Created by jaket on 28/05/2026.
//

#include "mtlx/node_evaluators/combine.h"

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

VarPtr evaluate_combine2(const TypePtr&, const vector<BasicValuePtr>& values)
{
    const auto& in1 = values[0];
    const auto& in2 = values[1];

    if (in1->is<int, float>() and in2->is<int, float>())
    {
        mx::Vector2 v{as_float(in1), as_float(in2)};
        return Variable::create(v);
    }

    if (in1->is<mx::Vector2>() and in2->is<mx::Vector2>())
    {
        mx::Vector2 v1 = in1->get<mx::Vector2>();
        mx::Vector2 v2 = in2->get<mx::Vector2>();
        mx::Vector4 v{v1[0], v1[1], v2[0], v2[1]};
        return Variable::create(v);
    }

    if (in1->is<mx::Vector3>() and in2->is<int, float>())
    {
        mx::Vector3 v1 = in1->get<mx::Vector3>();
        mx::Vector4 v{v1[0], v1[1], v1[2], as_float(in2)};
        return Variable::create(v);
    }

    if (in1->is<mx::Color3>() and in2->is<int, float>())
    {
        mx::Color3 c1 = in1->get<mx::Color3>();
        mx::Color4 c{c1[0], c1[1], c1[2], as_float(in2)};
        return Variable::create(c);
    }

    return nullptr;
}

VarPtr evaluate_combine3(const TypePtr& type, const vector<BasicValuePtr>& values)
{
    const auto& in1 = values[0];
    const auto& in2 = values[1];
    const auto& in3 = values[2];

    if (in1->is<int, float>() and in2->is<int, float>() and in3->is<int, float>())
    {
        if (type->is<mx::Vector3>())
        {
            mx::Vector3 v{as_float(in1), as_float(in2), as_float(in3)};
            return Variable::create(v);
        }
        if (type->is<mx::Color3>())
        {
            mx::Color3 c{as_float(in1), as_float(in2), as_float(in3)};
            return Variable::create(c);
        }
    }

    return nullptr;
}

VarPtr evaluate_combine4(const TypePtr& type, const vector<BasicValuePtr>& values)
{
    const auto& in1 = values[0];
    const auto& in2 = values[1];
    const auto& in3 = values[2];
    const auto& in4 = values[3];

    if (in1->is<int, float>() and in2->is<int, float>() and in3->is<int, float>() and in4->is<int, float>())
    {
        if (type->is<mx::Vector4>())
        {
            mx::Vector4 v{as_float(in1), as_float(in2), as_float(in3), as_float(in4)};
            return Variable::create(v);
        }
        if (type->is<mx::Color4>())
        {
            mx::Color4 c{as_float(in1), as_float(in2), as_float(in3), as_float(in4)};
            return Variable::create(c);
        }
    }

    return nullptr;
}
