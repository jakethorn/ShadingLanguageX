//
// Created by jaket on 03/01/2026.
//

#ifndef FENNEC_MTLX_UTILS_H
#define FENNEC_MTLX_UTILS_H

#include <MaterialXCore/Interface.h>
#include <MaterialXCore/Node.h>
#include <MaterialXCore/Document.h>

#include "common.h"
#include "constants.h"
#include "runtime/Type.h"

namespace mxslc::mtlx_utils
{
    mx::InputPtr add_or_get_input(const mx::NodePtr& node, const string& type, const string& name);
    mx::InputPtr add_or_get_input(const mx::NodePtr& node, const TypePtr& type, const string& name);
    mx::InputPtr add_or_get_input(const mx::NodeGraphPtr& node_graph, const TypePtr& type, const string& name);

    mx::OutputPtr add_or_get_output(const mx::NodeGraphPtr& node_graph, const TypePtr& type, const string& name);

    mx::NodeDefPtr get_node_def(const mx::NodePtr& node, const mx::DocumentPtr& mtlx_lib);
    mx::NodeDefPtr get_node_def(const mx::NodePtr& node, const string& mtlx_version = DEFAULT_MTLX_VERSION, const vector<fs::path>& include_dirs = {});
    mx::NodeDefPtr get_node_def(const mx::NodeGraphPtr& node_graph, const mx::DocumentPtr& mtlx_lib);
    mx::NodeDefPtr get_node_def(const mx::NodeGraphPtr& node_graph, const string& mtlx_version = DEFAULT_MTLX_VERSION, const vector<fs::path>& include_dirs = {});

    void set_interface(const mx::PortElementPtr& port, const string& interface_name);

    void remove_port(const mx::PortElementPtr& port);

    void validate(const mx::DocumentPtr& doc);

    mx::NodePtr create_dot_node(const mx::GraphElementPtr& graph, const TypePtr& type, const string& interface_name);
    mx::NodePtr create_dot_node(const mx::GraphElementPtr& graph, const string& name, const TypePtr& type, const string& interface_name);

    mx::NodePtr create_constant_node(const mx::GraphElementPtr& graph, const TypePtr& type);
    mx::NodePtr create_constant_node(const mx::GraphElementPtr& graph, const string& name, const TypePtr& type);

    template <typename T>
    mx::NodePtr create_constant_node(const mx::GraphElementPtr& graph, const TypePtr& type, const T& value)
    {
        return create_constant_node(graph, mx::EMPTY_STRING, type, value);
    }

    template <typename T>
    mx::NodePtr create_constant_node(const mx::GraphElementPtr& graph, const string& name, const TypePtr& type, const T& value)
    {
        const string valid_name = graph->createValidChildName(name);
        const mx::NodePtr node = graph->addNode("constant", valid_name, type->name());
        node->setInputValue("value", value);
        return node;
    }
}

#endif //FENNEC_MTLX_UTILS_H
