//
// Created by jaket on 03/01/2026.
//

#include "utils/mtlx_utils.h"

#include <algorithm>

#include <MaterialXFormat/XmlIo.h>

#include "runtime/Type.h"
#include "utils/string_utils.h"
#include "errors/CompileError.h"
#include "errors/MaterialXValidateError.h"
#include "utils/io_utils.h"
#include "utils/load_mtlx.h"
#include "utils/Logger.h"

namespace mxslc::mtlx_utils
{
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

    mx::InputPtr add_or_get_input(const mx::NodeGraphPtr& node_graph, const TypePtr& type, const string& name)
    {
        mx::InputPtr input = node_graph->getInput(name);
        if (not input)
            input = node_graph->addInput(name, type->name());

        return input;
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

    void set_interface(const mx::PortElementPtr& port, const string& interface_name)
    {
        port->removeAttribute("value");
        port->setInterfaceName(interface_name);
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
