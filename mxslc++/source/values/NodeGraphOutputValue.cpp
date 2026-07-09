//
// Created by jaket on 25/06/2026.
//

#include "values/NodeGraphOutputValue.h"

#include "runtime/Type.h"
#include "mtlx/mtlx_utils.h"
#include "values/interface.h"

namespace mxslc::values
{
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
        const mx::OutputPtr output = mtlx_utils::add_or_get_output(node_graph, type_, output_name);
        output->setOutputString(output_name_);
        output->setNodeGraphString(node_graph_name_);
    }

    string NodeGraphOutputValue::str() const
    {
        return "<output name=\"" + output_name_ + "\" type=\"" + type_->name() + "\" />";
    }
}
