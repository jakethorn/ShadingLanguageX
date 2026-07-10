//
// Created by jaket on 03/01/2026.
//

#include "serialize/mtlx_utils.h"
#include "runtime/Type.h"
#include "utils/string_utils.h"
#include "errors/CompileError.h"

namespace mxslc::mtlx_utils
{
    using namespace string_utils;

    string get_port_name(const string& port_name, const size_t i)
    {
        return port_name + "__" + str(i);
    }

    mx::InputPtr add_or_get_input(const mx::NodePtr& node, const string& type, const string& name)
    {
        if (mx::InputPtr input = node->getInput(name))
            return input;
        return node->addInput(name, type);
    }

    mx::InputPtr add_or_get_input(const mx::NodePtr& node, const TypePtr& type, const string& name)
    {
        return add_or_get_input(node, type->name(), name);
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

    mx::NodeDefPtr get_node_def(const mx::NodePtr& node, const mx::DocumentPtr& mtlx_lib)
    {
        mx::NodeDefPtr node_def = node->getNodeDef();
        if (node_def)
            return node_def;

        const mx::NodePtr copy = mtlx_lib->addNode(node->getCategory());
        copy->copyContentFrom(node);

        node_def = copy->getNodeDef();
        if (node_def)
        {
            mtlx_lib->removeNode(copy->getName());
            return node_def;
        }

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

    void set_interface(const mx::PortElementPtr& port, const string& interface_name)
    {
        port->removeAttribute("value");
        port->setInterfaceName(interface_name);
    }
}
