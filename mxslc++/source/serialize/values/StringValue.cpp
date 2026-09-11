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
        input->setValueString(value_);
    }

    void StringValue::set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const
    {
        // values cannot be given directly to outputs, so create a constant node as a passthrough
        const mx::NodePtr node = node_graph->addNode("constant", mx::EMPTY_STRING, type_->name());
        const mx::InputPtr input = node->addInput("value", type_->name());
        set_as_node_input(input);

        const mx::OutputPtr output = mtlx_utils::add_or_get_output(node_graph, type_, output_name);
        output->setConnectedNode(node);
    }

    void StringValue::set_as_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name) const
    {
        const mx::InputPtr input = node_def->addInput(input_name, type_->name());
        input->setValueString(value_);
    }

    void StringValue::set_as_node_graph_input(const mx::NodeGraphPtr& node_graph, const string& input_name) const
    {
        const mx::InputPtr input = node_graph->addInput(input_name, type_->name());
        input->setValueString(value_);
    }

    string StringValue::to_string() const
    {
        return value_;
    }
}
