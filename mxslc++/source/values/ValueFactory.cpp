//
// Created by jaket on 12/01/2026.
//

#include <cassert>

#include "ValueFactory.h"
#include "CastValue.h"
#include "runtime/Parameter.h"
#include "InterfaceValue.h"
#include "NodeValue.h"
#include "StructValue.h"
#include "mtlx/mtlx_utils.h"
#include "runtime/Variable.h"

namespace
{
    ValuePtr get_interface_from_type(const TypeInfoPtr& type, const string& name)
    {
        if (type->has_fields())
        {
            vector<ValuePtr> field_vals;
            for (size_t i = 0; i < type->field_count(); ++i)
            {
                ValuePtr field_val = get_interface_from_type(type->field_type(i), port_name(name, i));
                field_vals.push_back(std::move(field_val));
            }

            return std::make_shared<StructValue>(std::move(field_vals), type);
        }
        else
        {
            return std::make_shared<InterfaceValue>(name, type);
        }
    }
}

ValuePtr ValueFactory::create_parameter_interface(const Parameter& param)
{
    return get_interface_from_type(param.type(), param.name());
}

ValuePtr ValueFactory::create_nonlocal_interface(const VarPtr& var)
{
    const string output_name = "nonlocal_in__" + var->name();
    return get_interface_from_type(var->type(), output_name);
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

        return std::make_shared<StructValue>(std::move(subvalues), type);
    }
    else
    {
        return std::make_shared<OutputValue>(node, output_name, type);
    }
}

ValuePtr ValueFactory::cast_value(ValuePtr value, TypeInfoPtr type)
{
    assert(type->is_resolved());

    if (value->type()->is_equal(type))
        return value;
    else
        return std::make_shared<CastValue>(std::move(value), std::move(type));
}
