//
// Created by jaket on 25/06/2026.
//

#include "serialize/values/NodeGraphValue.h"

#include "runtime/Type.h"
#include "utils/mtlx_utils.h"
#include "serialize/values/interface.h"
#include "utils/string_utils.h"

namespace mxslc::serialize::values
{
    using string_utils::starts_with;

    NodeGraphValue::NodeGraphValue(const mx::NodeGraphPtr& node_graph) : NodeGraphValue{Type::of(node_graph), node_graph->getName()}
    {

    }

    NodeGraphValue::NodeGraphValue(TypePtr type, string node_graph_name) : Value{std::move(type)}, name_{std::move(node_graph_name)}
    {

    }

    bool NodeGraphValue::equals(const ValuePtr& other) const
    {
        if (const NodeGraphValuePtr other_node_graph = cast_value<NodeGraphValue>(other))
            return name_ == other_node_graph->name_;
        return false;
    }

    void NodeGraphValue::set_as_node_input(const mx::InputPtr& input) const
    {
        input->setNodeGraphString(name_);
    }

    void NodeGraphValue::set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const
    {
        // node graph strings cannot be given directly to outputs, so create a dot node as a passthrough
        const mx::NodePtr passthrough_node = create_passthrough_node(node_graph);

        const mx::OutputPtr output = mtlx_utils::add_or_get_output(node_graph, type_, output_name);
        output->setConnectedNode(passthrough_node);
    }

    void NodeGraphValue::set_as_node_graph_input(const mx::NodeGraphPtr& node_graph, const string& input_name) const
    {
        const mx::InputPtr input = mtlx_utils::add_or_get_input(node_graph, type_, input_name);
        input->setNodeGraphString(name_);
    }

    mx::NodePtr NodeGraphValue::create_passthrough_node(const mx::NodeGraphPtr& node_graph) const
    {
        const mx::NodePtr dot_node = node_graph->addNode("dot", mx::EMPTY_STRING, type_->name());
        const mx::InputPtr dot_node_input = dot_node->addInput("in", type_->name());
        dot_node_input->setNodeGraphString(name_);

        return dot_node;
    }

    string NodeGraphValue::to_string() const
    {
        string node_graph_name = name_;
        if (not starts_with(node_graph_name, "NG_"))
            node_graph_name = "NG_" + node_graph_name;

        return "nodegraph (" + node_graph_name + ")";
    }
}
