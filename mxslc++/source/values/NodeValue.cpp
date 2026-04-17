//
// Created by jaket on 11/04/2026.
//

#include "NodeValue.h"

#include "mtlx/mtlx_utils.h"

NodeValue::NodeValue(mx::NodePtr node, TypeInfoPtr type)
    : Value{std::move(type)}, node_{std::move(node)}
{

}

void NodeValue::set_as_node_input(const mx::NodePtr& node, const string& input_name) const
{
    node->setConnectedNode(input_name, node_);
}

void NodeValue::set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const
{
    const mx::OutputPtr output = add_or_get_output(node_graph, type_, output_name);
    output->setConnectedNode(node_);
}

string NodeValue::str() const
{
    return as_string(node_);
}
