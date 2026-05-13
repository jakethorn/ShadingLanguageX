//
// Created by jaket on 14/04/2026.
//

#include "OutputValue.h"

#include "runtime/Type.h"
#include "mtlx/mtlx_utils.h"

OutputValue::OutputValue(mx::OutputPtr output, TypePtr type) : Value{std::move(type)}, output_{std::move(output)} { }
OutputValue::OutputValue(const mx::NodePtr& node, const string& output_name, TypePtr type) : Value{std::move(type)}
{
    output_ = node->getOutput(output_name);
    if (output_ == nullptr)
    {
        const string& type_name = type_ == Type::Void ? Type::Int : type_->name();
        output_ = node->addOutput(output_name, type_name);
    }
}

bool OutputValue::equals(const ValuePtr& other) const
{
    if (const shared_ptr<OutputValue> other_output = std::dynamic_pointer_cast<OutputValue>(other))
        return output_ == other_output->output_;
    return false;
}

void OutputValue::set_as_node_input(const mx::InputPtr& input) const
{
    input->setConnectedOutput(output_);
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
