//
// Created by jaket on 14/04/2026.
//

#include "serialize/values/NodeOutputValue.h"

#include "runtime/Type.h"
#include "utils/mtlx_utils.h"
#include "serialize/values/interface.h"

namespace mxslc::serialize::values
{
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
        if (const NodeOutputValuePtr other_node_output = cast_value<NodeOutputValue>(other))
            return node_ == other_node_output->node_ and output_name_ == other_node_output->output_name_;
        return false;
    }

    void NodeOutputValue::set_as_node_input(const mx::InputPtr& input) const
    {
        input->setOutputString(output_name_);
        input->setConnectedNode(node_);
    }

    void NodeOutputValue::set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const
    {
        const mx::OutputPtr output = mtlx_utils::add_or_get_output(node_graph, type_, output_name);
        output->setOutputString(output_name_);
        output->setConnectedNode(node_);
    }

    string NodeOutputValue::to_string() const
    {
        if (mx::OutputPtr output = node_->getOutput(output_name_))
            return output->asString();
        else
            return "<output name=\"" + output_name_ + "\" type=\"" + type_->name() + "\" />";
    }
}
