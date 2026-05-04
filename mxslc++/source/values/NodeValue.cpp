//
// Created by jaket on 11/04/2026.
//

#include "NodeValue.h"

#include <cassert>

#include "mtlx/mtlx_utils.h"
#include "runtime/Type.h"

NodeValue::NodeValue(mx::NodePtr node) : Value{std::make_shared<ResolvedTypeInfo>(node->getType())}, node_{std::move(node)}
{
    assert(not node_->isMultiOutputType());
}

void NodeValue::set_as_node_input(const mx::InputPtr& input) const
{
    input->setConnectedNode(node_);
}

void NodeValue::set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const
{
    const mx::OutputPtr output = add_or_get_output(node_graph, type_, output_name);
    output->setConnectedNode(node_);
}

void NodeValue::set_node_name(const string& name) const
{
    node_->setName(
        node_->getParent()->createValidChildName(name)
    );
}

string NodeValue::str() const
{
    string str = node_->asString();
    for (const mx::InputPtr& i : node_->getInputs())
        str += "\n    "s + i->asString();
    for (const mx::OutputPtr& i : node_->getOutputs())
        str += "\n    "s + i->asString();
    return str;
}
