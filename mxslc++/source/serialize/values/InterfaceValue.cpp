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
    InterfaceValue::InterfaceValue(TypePtr type, string interface_name)
        : Value{std::move(type)}, interface_name_{std::move(interface_name)}
    {

    }

    bool InterfaceValue::equals(const ValuePtr& other) const
    {
        if (const InterfaceValuePtr other_interface = cast_value<InterfaceValue>(other))
            return interface_name_ == other_interface->interface_name_;
        return false;
    }

    void InterfaceValue::set_as_node_input(const mx::InputPtr& input) const
    {
        mtlx_utils::set_interface(input, interface_name_);
    }

    void InterfaceValue::set_as_node_graph_input(const mx::InputPtr& input) const
    {
        throw CompileError{"Invalid node graph input. You cannot reference variables from an enclosing function in a nodegraph function."};
    }

    void InterfaceValue::set_as_node_graph_output(const mx::OutputPtr& output) const
    {
        // interface names cannot be given directly to outputs, so create a dot node as a passthrough
        const mx::NodeGraphPtr node_graph = output->getParent()->asA<mx::NodeGraph>();
        const auto& [node, input] = mtlx_utils::create_dot(node_graph, type_);
        mtlx_utils::set_interface(input, interface_name_);

        mtlx_utils::set_connected_node(output, node);
    }

    string InterfaceValue::to_string() const
    {
        return "interface (" + interface_name_ + ")";
    }
}
