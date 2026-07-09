//
// Created by jaket on 25/06/2026.
//

#include "values/NodeGraphValue.h"

#include "mtlx/mtlx_utils.h"
#include "runtime/Type.h"
#include "values/interface.h"

namespace mxslc::values
{
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
        const mx::OutputPtr output = mtlx_utils::add_or_get_output(node_graph, type_, output_name);
        output->setNodeGraphString(name_);
    }

    string NodeGraphValue::str() const
    {
        return name_;
    }
}
