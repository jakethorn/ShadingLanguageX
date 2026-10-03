//
// Created by jaket on 11/09/2026.
//

#include "serialize/values/StringValue.h"

#include "runtime/Type.h"
#include "utils/mtlx_utils.h"
#include "serialize/values/interface.h"

namespace mxslc::serialize::values
{
    StringValue::StringValue(string value, TypePtr type) : Value{std::move(type)}, value_{std::move(value)} { }

    bool StringValue::equals(const ValuePtr& other) const
    {
        if (const StringValuePtr other_basic = cast_value<StringValue>(other))
            return value_ == other_basic->value_;
        return false;
    }

    void StringValue::set_as_node_input(const mx::InputPtr& input) const
    {
        mtlx_utils::set_value_string(input, value_);
    }

    void StringValue::set_as_node_def_input(const mx::InputPtr& input) const
    {
        mtlx_utils::set_value_string(input, value_);
    }

    void StringValue::set_as_node_graph_output(const mx::OutputPtr& output) const
    {
        // values cannot be given directly to outputs, so create a constant node as a passthrough
        const mx::NodeGraphPtr node_graph = output->getParent()->asA<mx::NodeGraph>();
        const auto& [node, input] = mtlx_utils::create_constant(node_graph, type_);
        mtlx_utils::set_value_string(input, value_);

        mtlx_utils::set_connected_node(output, node);
    }

    void StringValue::set_as_node_graph_input(const mx::InputPtr& input) const
    {
        mtlx_utils::set_value_string(input, value_);
    }

    string StringValue::to_string() const
    {
        return value_;
    }
}
