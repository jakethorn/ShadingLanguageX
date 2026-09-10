//
// Created by jaket on 25/06/2026.
//

#include "serialize/values/NodeGraphOutputValue.h"

#include "runtime/Type.h"
#include "utils/mtlx_utils.h"
#include "serialize/values/interface.h"
#include "utils/string_utils.h"

namespace mxslc::serialize::values
{
    using string_utils::starts_with;

    NodeGraphOutputValue::NodeGraphOutputValue(TypePtr type, const mx::NodeGraphPtr& node_graph, string output_name)
        : NodeGraphOutputValue{std::move(type), node_graph->getName(), std::move(output_name)}
    {

    }

    NodeGraphOutputValue::NodeGraphOutputValue(TypePtr type, string node_graph_name, string output_name)
        : Value{std::move(type)}, node_graph_name_{std::move(node_graph_name)}, output_name_{std::move(output_name)}
    {

    }

    bool NodeGraphOutputValue::equals(const ValuePtr& other) const
    {
        if (const NodeGraphOutputValuePtr other_node_graph_output = cast_value<NodeGraphOutputValue>(other))
            return node_graph_name_ == other_node_graph_output->node_graph_name_ and output_name_ == other_node_graph_output->output_name_;
        return false;
    }

    void NodeGraphOutputValue::set_as_node_input(const mx::InputPtr& input) const
    {
        input->setOutputString(output_name_);
        input->setNodeGraphString(node_graph_name_);
    }

    void NodeGraphOutputValue::set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const
    {
        // node graph outputs cannot be given directly to outputs, so create a dot node as a passthrough
        const mx::NodePtr passthrough_node = create_passthrough_node(node_graph);

        const mx::OutputPtr output = mtlx_utils::add_or_get_output(node_graph, type_, output_name);
        output->setConnectedNode(passthrough_node);
    }

    void NodeGraphOutputValue::set_as_node_graph_input(const mx::NodeGraphPtr& node_graph, const string& input_name) const
    {
        const mx::InputPtr input = mtlx_utils::add_or_get_input(node_graph, type_, input_name);
        input->setOutputString(output_name_);
        input->setNodeGraphString(node_graph_name_);
    }

    mx::NodePtr NodeGraphOutputValue::create_passthrough_node(const mx::NodeGraphPtr& node_graph) const
    {
        const mx::NodePtr dot_node = node_graph->addNode("dot", mx::EMPTY_STRING, type_->name());
        const mx::InputPtr dot_node_input = dot_node->addInput("in", type_->name());
        dot_node_input->setOutputString(output_name_);
        dot_node_input->setNodeGraphString(node_graph_name_);

        return dot_node;
    }

    string NodeGraphOutputValue::to_string() const
    {
        string node_graph_name = node_graph_name_;
        if (not starts_with(node_graph_name, "NG_"))
            node_graph_name = "NG_" + node_graph_name;

        return "nodegraph-output (" + node_graph_name + "." + output_name_ + ")";
    }
}
