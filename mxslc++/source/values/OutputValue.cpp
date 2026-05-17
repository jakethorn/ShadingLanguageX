//
// Created by jaket on 14/04/2026.
//

#include "OutputValue.h"

#include "runtime/Type.h"
#include "mtlx/mtlx_utils.h"

OutputValue::OutputValue(TypePtr type, mx::NodePtr node, string output_name)
    : Value{std::move(type)}, node_{std::move(node)}, output_name_{std::move(output_name)}
{

}


bool OutputValue::equals(const ValuePtr& other) const
{
    if (const shared_ptr<OutputValue> other_output = std::dynamic_pointer_cast<OutputValue>(other))
        return node_ == other_output->node_ and output_name_ == other_output->output_name_;
    return false;
}

void OutputValue::set_as_node_input(const mx::InputPtr& input) const
{
    input->setOutputString(output_name_);
    input->setConnectedNode(node_);
}

void OutputValue::set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const
{
    const mx::OutputPtr output = add_or_get_output(node_graph, type_, output_name);
    output->setOutputString(output_name_);
    output->setConnectedNode(node_);
}

string OutputValue::str() const
{
    return "<output name=\"" + output_name_ + "\" type=\"" + type_->name() + "\" />";
}
