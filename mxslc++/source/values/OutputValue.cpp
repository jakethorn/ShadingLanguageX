//
// Created by jaket on 14/04/2026.
//

#include "OutputValue.h"

#include "runtime/TypeInfo.h"
#include "mtlx/mtlx_utils.h"

OutputValue::OutputValue(mx::OutputPtr output, TypeInfoPtr type) : Value{std::move(type)}, output_{std::move(output)} { }
OutputValue::OutputValue(const mx::NodePtr& node, const string& output_name, TypeInfoPtr type) : Value{std::move(type)}
{
    output_ = node->getOutput(output_name);
    if (output_ == nullptr)
        output_ = node->addOutput(output_name, type_->name());
}

void OutputValue::set_as_node_input(const mx::NodePtr& node, const string& input_name) const
{
    node->setConnectedOutput(input_name, output_);
}

void OutputValue::set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const
{
    const mx::OutputPtr output = add_or_get_output(node_graph, type_, output_name);
    output->setConnectedOutput(output_);
}

string OutputValue::str() const
{
    return output_->asString();
}
