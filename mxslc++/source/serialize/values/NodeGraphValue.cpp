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

    NodeGraphValue::NodeGraphValue(TypePtr type, string node_graph_name) : Value{std::move(type)}, node_graph_name_{std::move(node_graph_name)}
    {

    }

    bool NodeGraphValue::equals(const ValuePtr& other) const
    {
        if (const NodeGraphValuePtr other_node_graph = cast_value<NodeGraphValue>(other))
            return node_graph_name_ == other_node_graph->node_graph_name_;
        return false;
    }

    void NodeGraphValue::set_as_node_input(const mx::InputPtr& input) const
    {
        mtlx_utils::set_node_graph_string(input, node_graph_name_);
    }

    void NodeGraphValue::set_as_node_graph_input(const mx::InputPtr& input) const
    {
        mtlx_utils::set_node_graph_string(input, node_graph_name_);
    }

    void NodeGraphValue::set_as_node_graph_output(const mx::OutputPtr& output) const
    {
        // node graph strings cannot be given directly to outputs, so create a dot node as a passthrough
        const mx::NodeGraphPtr node_graph = output->getParent()->asA<mx::NodeGraph>();
        const auto& [node, input] = mtlx_utils::create_dot(node_graph, type_);
        mtlx_utils::set_node_graph_string(input, node_graph_name_);

        mtlx_utils::set_connected_node(output, node);
    }

    string NodeGraphValue::to_string() const
    {
        string node_graph_name = node_graph_name_;
        if (not starts_with(node_graph_name, "NG_"))
            node_graph_name = "NG_" + node_graph_name;

        return "nodegraph (" + node_graph_name + ")";
    }
}
