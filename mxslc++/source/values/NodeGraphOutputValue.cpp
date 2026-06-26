//
// Created by jaket on 25/06/2026.
//

#include "NodeGraphOutputValue.h"

#include "runtime/Type.h"
#include "mtlx/mtlx_utils.h"

NodeGraphOutputValue::NodeGraphOutputValue(TypePtr type, mx::NodeGraphPtr node_graph, string output_name)
    : Value{std::move(type)}, node_graph_{std::move(node_graph)}, output_name_{std::move(output_name)}
{

}

bool NodeGraphOutputValue::equals(const ValuePtr& other) const
{
    if (const shared_ptr<NodeGraphOutputValue> other_output = std::dynamic_pointer_cast<NodeGraphOutputValue>(other))
        return node_graph_ == other_output->node_graph_ and output_name_ == other_output->output_name_;
    return false;
}

void NodeGraphOutputValue::set_as_node_input(const mx::InputPtr& input) const
{
    input->setOutputString(output_name_);
    input->setNodeGraphString(node_graph_->getName());
}

void NodeGraphOutputValue::set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const
{
    const mx::OutputPtr output = add_or_get_output(node_graph, type_, output_name);
    output->setOutputString(output_name_);
    output->setNodeGraphString(node_graph_->getName());
}

string NodeGraphOutputValue::str() const
{
    return "<output name=\"" + output_name_ + "\" type=\"" + type_->name() + "\" />";
}
