//
// Created by jaket on 03/01/2026.
//

#include "utils/mtlx_utils.h"

#include <MaterialXFormat/XmlIo.h>

#include "Primitive.h"
#include "runtime/Type.h"
#include "errors/CompileError.h"
#include "errors/MaterialXValidateError.h"
#include "utils/Logger.h"

namespace mxslc::mtlx_utils
{
    mx::NodePtr create_node(const mx::GraphElementPtr& graph, const string& type, const string& category)
    {
        return graph->addNode(category, mx::EMPTY_STRING, type);
    }

    mx::NodePtr create_node(const mx::GraphElementPtr& graph, const TypePtr& type, const string& category)
    {
        return create_node(graph, type->name(), category);
    }

    std::pair<mx::NodePtr, mx::InputPtr> create_dot(const mx::GraphElementPtr& graph, const TypePtr& type)
    {
        const mx::NodePtr node = create_node(graph, type, "dot");
        const mx::InputPtr input = add_or_get_input(node, type, "in");

        return {node, input};
    }

    std::pair<mx::NodePtr, mx::InputPtr> create_constant(const mx::GraphElementPtr& graph, const TypePtr& type)
    {
        const mx::NodePtr node = create_node(graph, type, "constant");
        const mx::InputPtr input = add_or_get_input(node, type, "value");

        return {node, input};
    }

    mx::NodePtr create_constant(const mx::GraphElementPtr& graph, const Primitive& value)
    {
        const auto& [node, input] = create_constant(graph, value.type());
        set_value(input, value);

        return node;
    }

    mx::InputPtr add_or_get_input(const mx::InterfaceElementPtr& element, const string& type, const string& name)
    {
        if (mx::InputPtr input = element->getInput(name))
            return input;
        return element->addInput(name, type);
    }

    mx::InputPtr add_or_get_input(const mx::InterfaceElementPtr& element, const TypePtr& type, const string& name)
    {
        return add_or_get_input(element, type->name(), name);
    }

    mx::OutputPtr add_or_get_output(const mx::NodeGraphPtr& node_graph, const TypePtr& type, const string& name)
    {
        const mx::NodeDefPtr node_def = node_graph->getNodeDef();
        if (node_def and not node_def->getOutput(name))
            node_def->addOutput(name, type->name());

        mx::OutputPtr output = node_graph->getOutput(name);
        if (not output)
            output = node_graph->addOutput(name, type->name());

        return output;
    }

    mx::NodeDefPtr get_node_def(const mx::NodePtr& node)
    {
        mx::NodeDefPtr node_def = node->getNodeDef();
        if (node_def)
            return node_def;

        throw CompileError{"Cannot find NodeDef for " + node->getCategory()};
    }

    mx::NodeDefPtr get_node_def(const mx::NodeGraphPtr& node_graph, const mx::DocumentPtr& mtlx_lib)
    {
        mx::NodeDefPtr node_def = node_graph->getNodeDef();
        if (node_def)
            return node_def;

        node_def = mtlx_lib->getNodeDef(node_graph->getNodeDefString());
        if (node_def)
            return node_def;

        throw CompileError{"Cannot find NodeDef for " + node_graph->getName()};
    }

    mx::NodeGraphPtr get_node_graph(const mx::NodeDefPtr& node_def)
    {
        for (const mx::NodeGraphPtr& node_graph : node_def->getDocument()->getNodeGraphs())
        {
            if (node_graph->getNodeDef() == node_def)
                return node_graph;
        }
        return nullptr;
    }

    void set_value(const mx::InputPtr& input, const Primitive& value)
    {
        clear_binding(input, value.to_string());

        value.visit([&input, &value](const auto& v) {
            IF_VISITED_TYPE_IS(std::monostate)
                remove_port(input);
            else IF_VISITED_TYPE_IS(fs::path)
                input->setValue(v.string(), value.type_name());
            else
                input->setValue(v, value.type_name());
        });
    }

    void set_value(const mx::InterfaceElementPtr& element, const string& input_name, const Primitive& value)
    {
        const mx::InputPtr input = add_or_get_input(element, value.type(), input_name);
        set_value(input, value);
    }

    void set_connected_node(const mx::PortElementPtr& port, const mx::NodePtr& node)
    {
        clear_binding(port, node->getName());
        port->setConnectedNode(node);
    }

    void set_connected_node_output(const mx::PortElementPtr& port, const mx::NodePtr& node, const string& output_name)
    {
        clear_binding(port, output_name);
        port->setOutputString(output_name);
        port->setConnectedNode(node);
    }

    void set_node_graph_string(const mx::PortElementPtr& port, const string& node_graph_name)
    {
        clear_binding(port, node_graph_name);
        port->setNodeGraphString(node_graph_name);
    }

    void set_node_graph_output_string(const mx::PortElementPtr& port, const string& node_graph_name, const string& output_name)
    {
        clear_binding(port, output_name);
        port->setOutputString(output_name);
        port->setNodeGraphString(node_graph_name);
    }

    void set_interface(const mx::PortElementPtr& port, const string& interface_name)
    {
        clear_binding(port, interface_name);
        port->setInterfaceName(interface_name);
    }

    void set_value_string(const mx::PortElementPtr& port, const string& value_string)
    {
        clear_binding(port, value_string);
        port->setValueString(value_string);
    }

    void clear_binding(const mx::PortElementPtr& port, const string& new_value)
    {
        if (new_value.empty())
            return;

        port->removeAttribute(mx::ValueElement::VALUE_ATTRIBUTE);
        port->removeAttribute(mx::PortElement::OUTPUT_ATTRIBUTE);
        port->removeAttribute(mx::PortElement::NODE_NAME_ATTRIBUTE);
        port->removeAttribute(mx::ValueElement::INTERFACE_NAME_ATTRIBUTE);
        port->removeAttribute(mx::PortElement::NODE_GRAPH_ATTRIBUTE);
    }

    void remove_port(const mx::PortElementPtr& port)
    {
        port->getParent()->removeChild(port->getName());
    }

    void validate(const mx::DocumentPtr& doc)
    {
        const auto [doc_major, doc_minor] = doc->getVersionIntegers();
        const auto [lib_major, lib_minor, lib_build] = mx::getVersionIntegers();
        if (doc_major == lib_major and doc_minor == lib_minor)
        {
            string message = mx::writeToXmlString(doc);
            if (not doc->validate(&message))
                throw MaterialXValidateError{message};
        }
        else
        {
            Logger::warning("Document version (" + doc->getVersionString() + ") is too old to be validated.");
        }
    }
}
