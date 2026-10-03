//
// Created by jaket on 03/01/2026.
//

#ifndef FENNEC_MTLX_UTILS_H
#define FENNEC_MTLX_UTILS_H

#include <MaterialXCore/Interface.h>
#include <MaterialXCore/Node.h>
#include <MaterialXCore/Document.h>

#include "common.h"

namespace mxslc
{
    class Primitive;
}

namespace mxslc::mtlx_utils
{
    mx::NodePtr create_node(const mx::GraphElementPtr& graph, const string& type, const string& category);
    mx::NodePtr create_node(const mx::GraphElementPtr& graph, const TypePtr& type, const string& category);
    std::pair<mx::NodePtr, mx::InputPtr> create_dot(const mx::GraphElementPtr& graph, const TypePtr& type);
    std::pair<mx::NodePtr, mx::InputPtr> create_constant(const mx::GraphElementPtr& graph, const TypePtr& type);
    mx::NodePtr create_constant(const mx::GraphElementPtr& graph, const Primitive& value);

    mx::InputPtr add_or_get_input(const mx::InterfaceElementPtr& element, const string& type, const string& name);
    mx::InputPtr add_or_get_input(const mx::InterfaceElementPtr& element, const TypePtr& type, const string& name);
    mx::OutputPtr add_or_get_output(const mx::NodeGraphPtr& node_graph, const TypePtr& type, const string& name);

    mx::NodeDefPtr get_node_def(const mx::NodePtr& node);
    mx::NodeDefPtr get_node_def(const mx::NodeGraphPtr& node_graph, const mx::DocumentPtr& mtlx_lib);
    mx::NodeGraphPtr get_node_graph(const mx::NodeDefPtr& node_def);

    void set_value(const mx::InputPtr& input, const Primitive& value);
    void set_value(const mx::InterfaceElementPtr& element, const string& input_name, const Primitive& value);
    void set_connected_node(const mx::PortElementPtr& port, const mx::NodePtr& node);
    void set_connected_node_output(const mx::PortElementPtr& port, const mx::NodePtr& node, const string& output_name);
    void set_node_graph_string(const mx::PortElementPtr& port, const string& node_graph_name);
    void set_node_graph_output_string(const mx::PortElementPtr& port, const string& node_graph_name, const string& output_name);
    void set_interface(const mx::PortElementPtr& port, const string& interface_name);
    void set_value_string(const mx::PortElementPtr& port, const string& value_string);

    void clear_binding(const mx::PortElementPtr& port, const string& new_value);
    void remove_port(const mx::PortElementPtr& port);

    void validate(const mx::DocumentPtr& doc);
}

#endif //FENNEC_MTLX_UTILS_H
