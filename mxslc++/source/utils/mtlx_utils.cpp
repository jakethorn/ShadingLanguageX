//
// Created by jaket on 03/01/2026.
//

#include "utils/mtlx_utils.h"
#include "runtime/Type.h"
#include "utils/string_utils.h"
#include "errors/CompileError.h"
#include "utils/io_utils.h"
#include "utils/load_mtlx.h"

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

    mx::NodeDefPtr get_node_def(const mx::NodePtr& node, const string& mtlx_version, const vector<fs::path>& include_dirs)
    {
        const mx::DocumentPtr mtlx_lib = get_materialx_library(
            mtlx_version,
            include_dirs.empty() ? io_utils::get_default_search_directories() : include_dirs
        );

        return get_node_def(node, mtlx_lib);
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

    mx::NodeDefPtr get_node_def(const mx::NodeGraphPtr& node_graph, const string& mtlx_version, const vector<fs::path>& include_dirs)
    {
        const mx::DocumentPtr mtlx_lib = get_materialx_library(
            mtlx_version,
            include_dirs.empty() ? io_utils::get_default_search_directories() : include_dirs
        );

        return get_node_def(node_graph, mtlx_lib);
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

    void convert_to_nodegraph(const mx::NodeDefPtr& node_def, const mx::NodeGraphPtr& node_graph)
    {
        
    }
}
