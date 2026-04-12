//
// Created by jaket on 12/01/2026.
//

#include <cassert>

#include "ValueFactory.h"

#include "BasicValue.h"
#include "CastValue.h"
#include "CompileError.h"
#include "runtime/Parameter.h"
#include "InterfaceValue.h"
#include "NodeValue.h"
#include "StructValue.h"
#include "mtlx/mtlx_utils.h"
#include "runtime/Variable.h"

ValuePtr ValueFactory::create_interface_value(TypeInfoPtr type, const string& name)
{
    if (type->has_fields())
    {
        vector<ValuePtr> field_vals;
        for (size_t i = 0; i < type->field_count(); ++i)
        {
            ValuePtr field_val = create_interface_value(type->field_type(i), port_name(name, i));
            field_vals.push_back(std::move(field_val));
        }

        return std::make_shared<StructValue>(std::move(field_vals), std::move(type));
    }
    else
    {
        return std::make_shared<InterfaceValue>(name, std::move(type));
    }
}

ValuePtr ValueFactory::create_interface_value(const Parameter& param)
{
    return create_interface_value(param.type(), param.name());
}

ValuePtr ValueFactory::create_interface_value(const VarPtr& var)
{
    const string output_name = "nonlocal_in__" + var->name();
    return create_interface_value(var->type(), output_name);
}

ValuePtr ValueFactory::create_node_value(mx::NodePtr node, TypeInfoPtr type)
{
    ValuePtr value = std::make_shared<NodeValue>(node, type);

    if (value->subvalue_count() > 0)
    {
        vector<ValuePtr> subvalues;
        subvalues.reserve(value->subvalue_count());
        for (size_t i = 0; i < value->subvalue_count(); ++i)
        {
            subvalues.push_back(std::move(value->subvalue(i)));
        }

        return std::make_shared<StructValue>(std::move(subvalues), type);
    }
    else
    {
        if (node->getOutputCount() == 0)
        {
            return value;
        }
        else
        {
            return std::make_shared<OutputValue>(node, "out"s, type);
        }
    }
}

ValuePtr ValueFactory::create_output_value(mx::NodePtr node, const string& output_name, TypeInfoPtr type)
{
    if (type->has_fields())
    {
        vector<ValuePtr> subvalues;
        subvalues.reserve(type->field_count());
        for (size_t i = 0; i < type->field_count(); ++i)
        {
            ValuePtr subvalue = std::make_shared<OutputValue>(node, port_name(output_name, i), type->field_type(i));
            subvalues.push_back(std::move(subvalue));
        }

        return std::make_shared<StructValue>(std::move(subvalues), std::move(type));
    }
    else
    {
        return std::make_shared<OutputValue>(node, output_name, std::move(type));
    }
}

ValuePtr ValueFactory::create_default_value(TypeInfoPtr type)
{
    if (type->has_fields())
    {
        vector<ValuePtr> subvalues;
        subvalues.reserve(type->field_count());
        for (size_t i = 0; i < type->field_count(); ++i)
        {
            ValuePtr subvalue = create_default_value(type->field_type(i));
            subvalues.push_back(std::move(subvalue));
        }

        return std::make_shared<StructValue>(std::move(subvalues), std::move(type));
    }

#define START_INIT basic_t value = ""s; if constexpr (false) { }
#define INIT_BASIC(t) else if (type->is<t>()) value = t{};

    START_INIT
    INIT_BASIC(bool)
    INIT_BASIC(int)
    INIT_BASIC(float)
    INIT_BASIC(string)
    INIT_BASIC(mx::Vector2)
    INIT_BASIC(mx::Vector3)
    INIT_BASIC(mx::Vector4)
    INIT_BASIC(mx::Color3)
    INIT_BASIC(mx::Color4)
    INIT_BASIC(mx::Matrix33)
    INIT_BASIC(mx::Matrix44)

#undef INIT_BASIC
#undef START_INIT

    return std::make_shared<BasicValue>(std::move(value));
}

ValuePtr ValueFactory::copy_value_from_port(const mx::PortElementPtr& port)
{
    TypeInfoPtr type = std::make_shared<ResolvedTypeInfo>(port->getType());

    if (port->hasInterfaceName())
    {
        return create_interface_value(std::move(type), port->getInterfaceName());
    }

    if (port->hasNodeName())
    {
        return create_node_value(port->getConnectedNode(), std::move(type));
    }

    if (port->hasOutputString())
    {
        return std::make_shared<OutputValue>(port->getConnectedOutput(), std::move(type));
    }

    if (port->hasValue())
    {
#define START_INIT const mx::ValuePtr& port_value = port->getValue(); basic_t value = ""s; if constexpr (false) { }
#define INIT_BASIC(t) else if (port_value->isA<t>()) value = port_value->asA<t>();

        START_INIT
        INIT_BASIC(bool)
        INIT_BASIC(int)
        INIT_BASIC(float)
        INIT_BASIC(string)
        INIT_BASIC(mx::Vector2)
        INIT_BASIC(mx::Vector3)
        INIT_BASIC(mx::Vector4)
        INIT_BASIC(mx::Color3)
        INIT_BASIC(mx::Color4)
        INIT_BASIC(mx::Matrix33)
        INIT_BASIC(mx::Matrix44)

#undef INIT_BASIC
#undef START_INIT

        return std::make_shared<BasicValue>(value);
    }

    throw CompileError{"Port does not have a value"s};
}

ValuePtr ValueFactory::cast_value(ValuePtr value, TypeInfoPtr type)
{
    assert(type->is_resolved());

    if (value->type()->is_equal(type))
        return value;
    else
        return std::make_shared<CastValue>(std::move(value), std::move(type));
}
