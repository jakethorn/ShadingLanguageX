//
// Created by jaket on 11/04/2026.
//

#include "serialize/values/NodeValue.h"

#include <cassert>

#include "utils/mtlx_utils.h"
#include "serialize/serialize_name_utils.h"
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

    void NodeValue::set_assigned_node_name(const string& variable_name) const
    {
        // renaming a node does not update the ports that connect to it, so only nodes without connections are renamed
        if (is_node_name_set_ or not is_temporary_node_name(node_->getName()) or not node_->getDownstreamPorts().empty())
            return;
        is_node_name_set_ = true;

        node_->setName(
            node_->getParent()->createValidChildName(assigned_node_name(variable_name))
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

        const mx::InputPtr input = mtlx_utils::add_or_get_input(node_graph, type_, input_name);
        input->setConnectedNode(node_);
    }

    string NodeValue::to_string() const
    {
        return "node (" + node_->getCategory() + ", " + node_->getName() + ")";
    }
}
