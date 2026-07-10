//
// Created by jaket on 14/04/2026.
//

#include "serialize/values/InterfaceValue.h"

#include "runtime/Type.h"
#include "utils/mtlx_utils.h"
#include "serialize/values/interface.h"

namespace mxslc::values
{
    using mtlx_utils::add_or_get_output;
    using mtlx_utils::set_interface;

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
        set_interface(input, name_);
    }

    void InterfaceValue::set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const
    {
        const mx::OutputPtr output = add_or_get_output(node_graph, type_, output_name);
        set_interface(output, name_);
    }

    string InterfaceValue::str() const
    {
        return "<input name=\"" + name_ + "\" type=\"" + type_->name() + "\" />";
    }
}
