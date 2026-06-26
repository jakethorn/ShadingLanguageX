//
// Created by jaket on 25/06/2026.
//

#include "NodeGraphValue.h"

#include <cassert>

#include "mtlx/mtlx_utils.h"
#include "runtime/Runtime.h"
#include "runtime/Type.h"

NodeGraphValue::NodeGraphValue(mx::NodeGraphPtr node_graph) : Value{Type::of(node_graph)}, node_graph_{std::move(node_graph)}
{

}

bool NodeGraphValue::equals(const ValuePtr& other) const
{
    if (const shared_ptr<NodeGraphValue> other_node = std::dynamic_pointer_cast<NodeGraphValue>(other))
        return node_graph_ == other_node->node_graph_;
    return false;
}

void NodeGraphValue::set_as_node_input(const mx::InputPtr& input) const
{
    input->setNodeGraphString(node_graph_->getName());
}

void NodeGraphValue::set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const
{
    const mx::OutputPtr output = add_or_get_output(node_graph, type_, output_name);
    output->setNodeGraphString(node_graph_->getName());
}

string NodeGraphValue::str() const
{
    return node_graph_->asString();
}
