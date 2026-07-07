//
// Created by jaket on 14/04/2026.
//

#include "values/NodeOutputValue.h"

#include "runtime/Type.h"
#include "mtlx/mtlx_utils.h"

NodeOutputValue::NodeOutputValue(TypePtr type, mx::NodePtr node, string output_name)
    : Value{std::move(type)}, node_{std::move(node)}, output_name_{std::move(output_name)}
{

}

void NodeOutputValue::set_node_name(const string& name) const
{
    if (is_node_name_set_)
        return;
    is_node_name_set_ = true;

    if (node_->getName() == name)
        return;

    node_->setName(
        node_->getParent()->createValidChildName(name)
    );
}

bool NodeOutputValue::equals(const ValuePtr& other) const
{
    if (const shared_ptr<NodeOutputValue> other_output = std::dynamic_pointer_cast<NodeOutputValue>(other))
        return node_ == other_output->node_ and output_name_ == other_output->output_name_;
    return false;
}

void NodeOutputValue::set_as_node_input(const mx::InputPtr& input) const
{
    input->setOutputString(output_name_);
    input->setConnectedNode(node_);
}

void NodeOutputValue::set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const
{
    const mx::OutputPtr output = add_or_get_output(node_graph, type_, output_name);
    output->setOutputString(output_name_);
    output->setConnectedNode(node_);
}

string NodeOutputValue::str() const
{
    return "<output name=\"" + output_name_ + "\" type=\"" + type_->name() + "\" />";
}
