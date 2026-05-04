//
// Created by jaket on 12/01/2026.
//

#include "ValueFactory.h"

#include <cassert>

#include "CompileError.h"
#include "InterfaceValue.h"
#include "NodeValue.h"
#include "OutputValue.h"
#include "mtlx/mtlx_utils.h"
#include "runtime/Function.h"
#include "runtime/Type.h"
#include "runtime/Variable.h"

VarPtr ValueFactory::create_interface_value(TypePtr type, const string& name)
{
    if (type->has_fields())
    {
        vector<VarPtr> field_values;
        for (size_t i = 0; i < type->field_count(); ++i)
        {
            VarPtr field_value = create_interface_value(type->field_type(i), get_port_name(name, i));
            field_values.push_back(std::move(field_value));
        }

        return Variable::create(std::move(type), field_values);
    }
    else
    {
        ValuePtr value = std::make_shared<InterfaceValue>(std::move(type), name);
        return Variable::create(std::move(value));
    }
}

VarPtr ValueFactory::create_node_value(mx::NodePtr node, const mx::NodeDefPtr& node_def, TypePtr type)
{
    if (node_def->getOutputCount() > 1)
    {
        return create_output_value(std::move(node), std::move(type), "out"s);
    }
    else
    {
        ValuePtr value = std::make_shared<NodeValue>(std::move(node));
        return Variable::create(std::move(value));
    }
}

VarPtr ValueFactory::create_node_value(mx::NodePtr node, const FuncPtr& func)
{
    if (func->is_defined() or func->node_def()->getActiveOutputs().size() == 1)
    {
        return create_node_value(std::move(node), func->node_def(), func->return_type());
    }
    else
    {
        return create_output_values(std::move(node), func->return_type(), func->output_names());
    }
}

VarPtr ValueFactory::create_output_value(mx::NodePtr node, TypePtr type, const string& output_name)
{
    if (type->has_fields())
    {
        vector<VarPtr> field_values;
        field_values.reserve(type->field_count());
        for (size_t i = 0; i < type->field_count(); ++i)
        {
            VarPtr field_value = create_output_value(node, type->field_type(i), get_port_name(output_name, i));
            field_values.push_back(std::move(field_value));
        }

        return Variable::create(std::move(type), field_values);
    }
    else
    {
        ValuePtr value = std::make_shared<OutputValue>(std::move(node), output_name, std::move(type));
        return Variable::create(std::move(value));
    }
}

VarPtr ValueFactory::create_output_values(mx::NodePtr node, TypePtr type, const vector<string>& output_names)
{
    assert(type->field_count() == output_names.size());
    vector<VarPtr> field_values;
    field_values.reserve(type->field_count());
    for (size_t i = 0; i < type->field_count(); ++i)
    {
        VarPtr field_value = create_output_value(node, type->field_type(i), output_names[i]);
        field_values.push_back(std::move(field_value));
    }
    return Variable::create(std::move(type), field_values);
}

VarPtr ValueFactory::create_default_value(TypePtr type)
{
    if (type->has_fields())
    {
        vector<VarPtr> fields_values;
        fields_values.reserve(type->field_count());
        for (size_t i = 0; i < type->field_count(); ++i)
        {
            VarPtr field_value = create_default_value(type->field_type(i));
            fields_values.push_back(std::move(field_value));
        }

        return Variable::create(std::move(type), fields_values);
    }

#define START_INIT primitive_t primitive_value = ""s; if constexpr (false) { }
#define INIT_PRIM(t) else if (type->is<t>()) primitive_value = t{};

    START_INIT
    INIT_PRIM(bool)
    INIT_PRIM(int)
    INIT_PRIM(float)
    INIT_PRIM(string)
    INIT_PRIM(mx::Vector2)
    INIT_PRIM(mx::Vector3)
    INIT_PRIM(mx::Vector4)
    INIT_PRIM(mx::Color3)
    INIT_PRIM(mx::Color4)
    INIT_PRIM(mx::Matrix33)
    INIT_PRIM(mx::Matrix44)

#undef INIT_PRIM
#undef START_INIT

    ValuePtr value = std::make_shared<BasicValue>(std::move(primitive_value));
    return Variable::create(std::move(value));
}

ValuePtr ValueFactory::copy_value_from_port(const mx::PortElementPtr& port)
{
    TypePtr type = std::make_shared<ResolvedTypeInfo>(port->getType());

    if (port->hasInterfaceName())
    {
        return std::make_shared<InterfaceValue>(std::move(type), port->getInterfaceName());
    }

    if (port->hasOutputString())
    {
        return std::make_shared<OutputValue>(port->getConnectedOutput(), std::move(type));
    }

    if (port->hasNodeName())
    {
        return std::make_shared<NodeValue>(port->getConnectedNode());
    }

    if (port->hasValue())
    {
#define START_INIT const mx::ValuePtr& port_value = port->getValue(); primitive_t primitive_value = ""s; if constexpr (false) { }
#define INIT_PRIM(t) else if (port_value->isA<t>()) primitive_value = port_value->asA<t>();

        START_INIT
        INIT_PRIM(bool)
        INIT_PRIM(int)
        INIT_PRIM(float)
        INIT_PRIM(string)
        INIT_PRIM(mx::Vector2)
        INIT_PRIM(mx::Vector3)
        INIT_PRIM(mx::Vector4)
        INIT_PRIM(mx::Color3)
        INIT_PRIM(mx::Color4)
        INIT_PRIM(mx::Matrix33)
        INIT_PRIM(mx::Matrix44)

#undef INIT_PRIM
#undef START_INIT

        return std::make_shared<BasicValue>(primitive_value);
    }

    throw CompileError{"Port does not have a value"s};
}

VarPtr ValueFactory::create_default_value_impl(primitive_t value)
{
    return Variable::create(
        std::make_shared<BasicValue>(std::move(value))
    );
}
