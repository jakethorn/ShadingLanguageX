//
// Created by jaket on 14/04/2026.
//

#include "serialize/values/CompileTimeValue.h"

#include "runtime/Type.h"
#include "utils/mtlx_utils.h"
#include "serialize/values/interface.h"

namespace mxslc::serialize::values
{
    CompileTimeValue::CompileTimeValue(const mx::ValuePtr& value) : CompileTimeValue{Primitive{value}} { }
    CompileTimeValue::CompileTimeValue(Primitive value) : Value{value.type()}, value_{std::move(value)} { }
    CompileTimeValue::CompileTimeValue(Primitive value, TypePtr type) : Value{std::move(type)}, value_{std::move(value)} { }

    bool CompileTimeValue::equals(const ValuePtr& other) const
    {
        if (const CompileTimeValuePtr other_basic = cast_value<CompileTimeValue>(other))
            return (value_ == other_basic->value_).as<bool>();
        return false;
    }

    void CompileTimeValue::set_as_node_input(const mx::InputPtr& input) const
    {
        mtlx_utils::set_value(input, value_);
    }

    void CompileTimeValue::set_as_node_graph_input(const mx::InputPtr& input) const
    {
        mtlx_utils::set_value(input, value_);
    }

    void CompileTimeValue::set_as_node_graph_output(const mx::OutputPtr& output) const
    {
        // values cannot be given directly to outputs, so create a constant node as a passthrough
        const mx::NodeGraphPtr node_graph = output->getParent()->asA<mx::NodeGraph>();
        const mx::NodePtr constant_node = mtlx_utils::create_constant(node_graph, value_);

        mtlx_utils::set_connected_node(output, constant_node);
    }

    void CompileTimeValue::set_as_node_def_input(const mx::InputPtr& input) const
    {
        mtlx_utils::set_value(input, value_);
    }

    string CompileTimeValue::to_string() const
    {
        return value_.to_string();
    }
}
