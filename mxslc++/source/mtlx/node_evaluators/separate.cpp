//
// Created by jaket on 28/05/2026.
//

#include "separate.h"
#include "runtime/Type.h"
#include "runtime/Variable.h"
#include "values/BasicValue.h"

VarPtr evaluate_separate2(const TypePtr& type, const vector<BasicValuePtr>& values)
{
    const BasicValuePtr& in = values[0];
    const mx::Vector2 v = in->get<mx::Vector2>();
    VarPtr outx = Variable::create(v[0]);
    VarPtr outy = Variable::create(v[1]);
    return Variable::create(type, vector{std::move(outx), std::move(outy)});
}

VarPtr evaluate_separate3(const TypePtr& type, const vector<BasicValuePtr>& values)
{
    const BasicValuePtr& in = values[0];

    if (in->is<mx::Vector3>())
    {
        const mx::Vector3 v = in->get<mx::Vector3>();
        VarPtr outx = Variable::create(v[0]);
        VarPtr outy = Variable::create(v[1]);
        VarPtr outz = Variable::create(v[2]);
        return Variable::create(type, vector{std::move(outx), std::move(outy), std::move(outz)});
    }

    if (in->is<mx::Color3>())
    {
        const mx::Color3 c = in->get<mx::Color3>();
        VarPtr outr = Variable::create(c[0]);
        VarPtr outg = Variable::create(c[1]);
        VarPtr outb = Variable::create(c[2]);
        return Variable::create(type, vector{std::move(outr), std::move(outg), std::move(outb)});
    }

    return nullptr;
}

VarPtr evaluate_separate4(const TypePtr& type, const vector<BasicValuePtr>& values)
{
    const BasicValuePtr& in = values[0];

    if (in->is<mx::Vector4>())
    {
        const mx::Vector4 v = in->get<mx::Vector4>();
        VarPtr outx = Variable::create(v[0]);
        VarPtr outy = Variable::create(v[1]);
        VarPtr outz = Variable::create(v[2]);
        VarPtr outw = Variable::create(v[3]);
        return Variable::create(type, vector{std::move(outx), std::move(outy), std::move(outz), std::move(outw)});
    }

    if (in->is<mx::Color4>())
    {
        const mx::Color4 c = in->get<mx::Color4>();
        VarPtr outr = Variable::create(c[0]);
        VarPtr outg = Variable::create(c[1]);
        VarPtr outb = Variable::create(c[2]);
        VarPtr outa = Variable::create(c[2]);
        return Variable::create(type, vector{std::move(outr), std::move(outg), std::move(outb), std::move(outa)});
    }

    return nullptr;
}
