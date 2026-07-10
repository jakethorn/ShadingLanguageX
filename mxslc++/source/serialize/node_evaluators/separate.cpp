//
// Created by jaket on 28/05/2026.
//

#include "serialize/node_evaluators/separate.h"

#include "runtime/Type.h"
#include "runtime/interface.h"
#include "serialize/values/BasicValue.h"

namespace mxslc::serialize
{
    VarPtr evaluate_separate2(const TypePtr& type, const vector<BasicValuePtr>& values)
    {
        const BasicValuePtr& in = values[0];
        const mx::Vector2 v = in->get<mx::Vector2>();
        VarPtr outx = create_variable(v[0]);
        VarPtr outy = create_variable(v[1]);
        return create_variable(type, vector{std::move(outx), std::move(outy)});
    }

    VarPtr evaluate_separate3(const TypePtr& type, const vector<BasicValuePtr>& values)
    {
        const BasicValuePtr& in = values[0];

        if (in->is<mx::Vector3>())
        {
            const mx::Vector3 v = in->get<mx::Vector3>();
            VarPtr outx = create_variable(v[0]);
            VarPtr outy = create_variable(v[1]);
            VarPtr outz = create_variable(v[2]);
            return create_variable(type, vector{std::move(outx), std::move(outy), std::move(outz)});
        }

        if (in->is<mx::Color3>())
        {
            const mx::Color3 c = in->get<mx::Color3>();
            VarPtr outr = create_variable(c[0]);
            VarPtr outg = create_variable(c[1]);
            VarPtr outb = create_variable(c[2]);
            return create_variable(type, vector{std::move(outr), std::move(outg), std::move(outb)});
        }

        return nullptr;
    }

    VarPtr evaluate_separate4(const TypePtr& type, const vector<BasicValuePtr>& values)
    {
        const BasicValuePtr& in = values[0];

        if (in->is<mx::Vector4>())
        {
            const mx::Vector4 v = in->get<mx::Vector4>();
            VarPtr outx = create_variable(v[0]);
            VarPtr outy = create_variable(v[1]);
            VarPtr outz = create_variable(v[2]);
            VarPtr outw = create_variable(v[3]);
            return create_variable(type, vector{std::move(outx), std::move(outy), std::move(outz), std::move(outw)});
        }

        if (in->is<mx::Color4>())
        {
            const mx::Color4 c = in->get<mx::Color4>();
            VarPtr outr = create_variable(c[0]);
            VarPtr outg = create_variable(c[1]);
            VarPtr outb = create_variable(c[2]);
            VarPtr outa = create_variable(c[2]);
            return create_variable(type, vector{std::move(outr), std::move(outg), std::move(outb), std::move(outa)});
        }

        return nullptr;
    }
}

