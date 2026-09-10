//
// Created by jaket on 14/04/2026.
//

#include "serialize/values/InterfaceValue.h"

#include "runtime/Type.h"
#include "utils/mtlx_utils.h"
#include "serialize/values/interface.h"
#include "serialize/values/NodeValue.h"

namespace mxslc::serialize::values
{
    InterfaceValue::InterfaceValue(TypePtr type, string name)
        : Value{std::move(type)}, name_{std::move(name)}
    {

    }

    bool InterfaceValue::equals(const ValuePtr& other) const
    {
        if (const InterfaceValuePtr other_interface = cast_value<InterfaceValue>(other))
            return name_ == other_interface->name_;
        return false;
    }

    void InterfaceValue::set_as_node_input(const mx::InputPtr& input) const
    {
        mtlx_utils::set_interface(input, name_);
    }

    void InterfaceValue::set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const
    {
        // interface names cannot be given directly to outputs, so create a dot node as a passthrough
        const mx::NodePtr passthrough_node = create_passthrough_node(node_graph);

        const mx::OutputPtr output = mtlx_utils::add_or_get_output(node_graph, type_, output_name);
        output->setConnectedNode(passthrough_node);
    }

    void InterfaceValue::set_as_node_graph_input(const mx::NodeGraphPtr& node_graph, const string& input_name) const
    {
        throw CompileError{"Invalid node graph input. You cannot reference variables from an enclosing function in a nodegraph function."};
    }

    mx::NodePtr InterfaceValue::create_passthrough_node(const mx::NodeGraphPtr& node_graph) const
    {
        const mx::NodePtr dot_node = node_graph->addNode("dot", mx::EMPTY_STRING, type_->name());
        const mx::InputPtr dot_node_input = dot_node->addInput("in", type_->name());
        dot_node_input->setInterfaceName(name_);

        return dot_node;
    }

    string InterfaceValue::to_string() const
    {
        return "interface (" + name_ + ")";
    }
}
