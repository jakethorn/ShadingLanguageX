//
// Created by jaket on 10/07/2026.
//

#include "serialize/serializer_utils.h"

#include <cassert>

#include "runtime/Function.h"
#include "runtime/interface.h"
#include "runtime/Type.h"
#include "serialize/serialize_name_utils.h"
#include "serialize/values/interface.h"
#include "serialize/values/InterfaceValue.h"
#include "serialize/values/NodeGraphOutputValue.h"
#include "serialize/values/NodeGraphValue.h"
#include "serialize/values/NodeOutputValue.h"
#include "serialize/values/NodeValue.h"
#include "serialize/values/CompileTimeValue.h"
#include "utils/mtlx_utils.h"

namespace mxslc::serialize_utils
{
    VarPtr create_interface_value(TypePtr type, const string& name)
    {
        if (type->has_fields())
        {
            vector<VarPtr> field_values;
            for (size_t i = 0; i < type->field_count(); ++i)
            {
                VarPtr field_value = create_interface_value(type->field_type(i), with_prefix(name, type, i));
                field_values.push_back(std::move(field_value));
            }

            return create_variable(std::move(type), field_values);
        }
        else
        {
            ValuePtr value = create_value<InterfaceValue>(std::move(type), name);
            return create_variable(std::move(value));
        }
    }

    VarPtr create_node_value(mx::NodePtr node, const mx::NodeDefPtr& node_def, TypePtr type)
    {
        if (node_def->getOutputCount() > 1)
        {
            return create_node_output_value(std::move(node), std::move(type), RETURN_VALUE_PREFIX);
        }
        else
        {
            ValuePtr value = create_value<NodeValue>(std::move(node));
            return create_variable(std::move(value));
        }
    }

    VarPtr create_node_value(mx::NodePtr node, const FuncPtr& func)
    {
        if (func->is_defined() or func->node_def()->getActiveOutputs().size() == 1)
        {
            return create_node_value(std::move(node), func->node_def(), func->return_type());
        }
        else
        {
            return create_node_output_value(std::move(node), func->return_type(), func->output_names());
        }
    }

    VarPtr create_node_graph_value(mx::NodeGraphPtr node_graph, TypePtr type)
    {
        if (node_graph->getOutputCount() > 1)
        {
            return create_node_graph_output_value(std::move(node_graph), std::move(type), RETURN_VALUE_PREFIX);
        }
        else
        {
            ValuePtr value = create_value<NodeGraphValue>(std::move(node_graph));
            return create_variable(std::move(value));
        }
    }

    VarPtr create_node_graph_value(const FuncPtr& func)
    {
        return create_node_graph_value(func->node_graph(), func->return_type());
    }

    VarPtr create_node_output_value(mx::NodePtr node, TypePtr type, const string& output_name)
    {
        return create_node_output_value(std::move(node), std::move(type), output_name, AttributeList{});
    }

    VarPtr create_node_output_value(mx::NodePtr node, TypePtr type, const string& output_name, const AttributeList& attrs)
    {
        if (type->has_fields())
        {
            vector<VarPtr> field_values;
            field_values.reserve(type->field_count());
            for (size_t i = 0; i < type->field_count(); ++i)
            {
                VarPtr field_value = create_node_output_value(node, type->field_type(i), with_prefix(output_name, type, i), attrs);
                field_values.push_back(std::move(field_value));
            }

            return create_variable(std::move(type), field_values);
        }
        else
        {
            attrs.add_to(node, output_name);
            ValuePtr value = create_value<NodeOutputValue>(std::move(type), std::move(node), output_name);
            return create_variable(std::move(value));
        }
    }

    VarPtr create_node_output_value(mx::NodePtr node, TypePtr type, const vector<string>& output_names)
    {
        assert(type->field_count() == output_names.size());
        vector<VarPtr> field_values;
        field_values.reserve(type->field_count());
        for (size_t i = 0; i < type->field_count(); ++i)
        {
            VarPtr field_value = create_node_output_value(node, type->field_type(i), output_names[i]);
            field_values.push_back(std::move(field_value));
        }
        return create_variable(std::move(type), field_values);
    }

    VarPtr create_node_graph_output_value(mx::NodeGraphPtr node_graph, TypePtr type, const string& output_name)
    {
        if (type->has_fields())
        {
            vector<VarPtr> field_values;
            field_values.reserve(type->field_count());
            for (size_t i = 0; i < type->field_count(); ++i)
            {
                VarPtr field_value = create_node_graph_output_value(node_graph, type->field_type(i), with_prefix(output_name, type, i));
                field_values.push_back(std::move(field_value));
            }

            return create_variable(std::move(type), field_values);
        }
        else
        {
            ValuePtr value = create_value<NodeGraphOutputValue>(std::move(type), std::move(node_graph), output_name);
            return create_variable(std::move(value));
        }
    }

    VarPtr create_compile_time_value(Primitive value)
    {
        return create_variable(std::move(value));
    }

    VarPtr create_compile_time_value(TypePtr type)
    {
        if (type->is_auto())
            throw CompileError{"Cannot create default value for variable of type auto"};

        if (type->has_fields())
        {
            vector<VarPtr> fields_values;
            fields_values.reserve(type->field_count());
            for (size_t i = 0; i < type->field_count(); ++i)
            {
                VarPtr field_value = create_compile_time_value(type->field_type(i));
                fields_values.push_back(std::move(field_value));
            }

            return create_variable(std::move(type), fields_values);
        }

        return create_variable(Primitive(type));
    }

    ValuePtr copy_value_from_port(const mx::PortElementPtr& port)
    {
        if (port->hasInterfaceName())
        {
            return create_value<InterfaceValue>(Type::of(port), port->getInterfaceName());
        }

        if (port->hasNodeName())
        {
            if (port->hasOutputString())
            {
                return create_value<NodeOutputValue>(Type::of(port), port->getConnectedNode(), port->getOutputString());
            }

            return create_value<NodeValue>(port->getConnectedNode());
        }

        if (port->hasNodeGraphString())
        {
            if (port->hasOutputString())
            {
                return create_value<NodeGraphOutputValue>(Type::of(port), port->getNodeGraphString(), port->getOutputString());
            }

            return create_value<NodeGraphValue>(Type::of(port), port->getNodeGraphString());
        }

        if (port->hasValue())
        {
            return create_value<CompileTimeValue>(port->getValue());
        }

        throw CompileError{"Port does not have a value"};
    }
}
