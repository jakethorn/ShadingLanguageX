//
// Created by jaket on 11/04/2026.
//

#include "serialize/values/NodeValue.h"

#include <cassert>

#include "utils/mtlx_utils.h"
#include "serialize/values/interface.h"
#include "runtime/Type.h"

namespace mxslc::serialize::values
{
    NodeValue::NodeValue(mx::NodePtr node) : Value{Type::of(node)}, node_{std::move(node)}
    {
        assert(not node_->isMultiOutputType());
    }

    void NodeValue::set_node_name(const string& name) const
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

    bool NodeValue::equals(const ValuePtr& other) const
    {
        if (const NodeValuePtr other_node = cast_value<NodeValue>(other))
            return node_ == other_node->node_;
        return false;
    }

    void NodeValue::set_as_node_input(const mx::InputPtr& input) const
    {
        input->setConnectedNode(node_);
    }

    void NodeValue::set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const
    {
        const mx::OutputPtr output = mtlx_utils::add_or_get_output(node_graph, type_, output_name);
        output->setConnectedNode(node_);
    }

    string NodeValue::to_string() const
    {
        string str = node_->asString();
        for (const mx::InputPtr& i : node_->getInputs())
            str += "\n    "s + i->asString();
        for (const mx::OutputPtr& i : node_->getOutputs())
            str += "\n    "s + i->asString();
        return str;
    }
}
