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
        input->removeAttribute("value");
        input->setConnectedNode(node_);
    }

    void NodeValue::set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const
    {
        const mx::OutputPtr output = mtlx_utils::add_or_get_output(node_graph, type_, output_name);
        output->setConnectedNode(node_);
    }

    void NodeValue::set_as_node_graph_input(const mx::NodeGraphPtr& node_graph, const string& input_name) const
    {
        if (node_graph->getParent() != node_->getParent())
            throw CompileError{"Invalid node graph input. You cannot reference variables from an enclosing function in a nodegraph function."};

        const mx::InputPtr input = node_graph->addInput(input_name);
        input->setConnectedNode(node_);
    }

    string NodeValue::to_string() const
    {
        return "node (" + node_->getCategory() + ", " + node_->getName() + ")";
    }
}
