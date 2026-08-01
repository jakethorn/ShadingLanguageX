//
// Created by jaket on 08/07/2026.
//

#include "runtime/interface.h"

#include "runtime/Variable.h"
#include "runtime/utils/type_utils.h"
#include "serialize/values/interface.h"
#include "serialize/serializer_utils.h"
#include "serialize/values/Value.h"
#include "serialize/values/BasicValue.h"

namespace mxslc::runtime
{
    using namespace type_utils;

    class VariableFactory
    {
    public:
        static void copy_children(const VarPtr& var, const vector<VarPtr>& children)
        {
            var->copy_children(children);
        }

        static void copy_value(const VarPtr& var, ValuePtr value)
        {
            var->copy_value(std::move(value));
        }
    };

    VarPtr create_variable(ModifierList mods, TypePtr type, const vector<VarPtr>& children)
    {
        if (contains_auto(type))
            type = replace_auto(type, type_of(children));

        VarPtr var = std::make_shared<Variable>(std::move(mods), std::move(type));
        VariableFactory::copy_children(var, children);
        return var;
    }

    VarPtr create_variable(ModifierList mods, TypePtr type, const vector<Primitive>& children)
    {
        vector<VarPtr> vars;
        vars.reserve(children.size());
        for (const Primitive& child : children)
            vars.push_back(create_variable(child));

        return create_variable(std::move(mods), std::move(type), vars);
    }

    VarPtr create_variable(ModifierList mods, TypePtr type, ValuePtr value)
    {
        if (contains_auto(type))
            type = replace_auto(type, value->type());

        VarPtr var = std::make_shared<Variable>(std::move(mods), std::move(type));
        VariableFactory::copy_value(var, std::move(value));
        return var;
    }

    VarPtr create_variable(ModifierList mods, TypePtr type, const VarPtr& value)
    {
        if (contains_auto(type))
            type = replace_auto(type, value->type());

        VarPtr var = std::make_shared<Variable>(std::move(mods), std::move(type));
        var->copy(value);
        return var;
    }

    VarPtr create_variable(ModifierList mods, ValuePtr value)
    {
        TypePtr type = value->type();
        return create_variable(std::move(mods), std::move(type), std::move(value));
    }

    VarPtr create_variable(ModifierList mods, Primitive value)
    {
        BasicValuePtr basic_value = create_value(std::move(value));
        return create_variable(std::move(mods), std::move(basic_value));
    }

    VarPtr create_variable(TypePtr type, const vector<VarPtr>& children)
    {
        return create_variable(ModifierList{}, std::move(type), children);
    }

    VarPtr create_variable(TypePtr type, ValuePtr value)
    {
        return create_variable(ModifierList{}, std::move(type), std::move(value));
    }

    VarPtr create_variable(TypePtr type, const VarPtr& value)
    {
        return create_variable(ModifierList{}, std::move(type), value);
    }

    VarPtr create_variable(TypePtr type)
    {
        return serialize_utils::create_basic_value(std::move(type));
    }

    VarPtr create_variable(const vector<VarPtr>& children)
    {
        return create_variable(ModifierList{}, type_of(children), children);
    }

    VarPtr create_variable(const vector<Primitive>& children)
    {
        vector<VarPtr> vars;
        vars.reserve(children.size());
        for (const Primitive& child : children)
            vars.push_back(create_variable(child));

        return create_variable(vars);
    }

    VarPtr create_variable(ValuePtr value)
    {
        TypePtr type = value->type();
        return create_variable(ModifierList{}, std::move(type), std::move(value));
    }

    VarPtr create_variable(Primitive value)
    {
        BasicValuePtr basic_value = create_value<BasicValue>(std::move(value));
        return create_variable(std::move(basic_value));
    }

    VarPtr create_variable(const VarPtr& value)
    {
        VarPtr copy = create_variable(ModifierList{}, value->type(), value);
        if (value->is_external())
            copy->set_is_external();
        return copy;
    }
}
