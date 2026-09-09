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
        const mx::NodePtr dot_node = mtlx_utils::create_dot_node(node_graph, output_name + "_interface", type_, name_);

        const mx::OutputPtr output = mtlx_utils::add_or_get_output(node_graph, type_, output_name);
        output->setConnectedNode(dot_node);
    }

    void InterfaceValue::set_as_node_graph_input(const mx::NodeGraphPtr& node_graph, const string& input_name) const
    {
        throw CompileError{"Invalid node graph input. You cannot reference variables from an enclosing function in a nodegraph function."};
    }

    string InterfaceValue::to_string() const
    {
        return "interface (" + name_ + ")";
    }
}
