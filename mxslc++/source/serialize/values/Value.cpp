//
// Created by jaket on 26/03/2026.
//

#include "serialize/values/Value.h"

#include <cassert>

#include "utils/mtlx_utils.h"
#include "runtime/Type.h"

namespace mxslc::serialize::values
{
    Value::Value(TypePtr type) : type_{std::move(type)}
    {
        assert(type_->is_resolved());
        assert(type_->is_primitive());
    }

    void Value::set_as_node_input(const mx::NodePtr& node, const string& input_name) const
    {
        const mx::InputPtr input = mtlx_utils::add_or_get_input(node, type_, input_name);
        set_as_node_input(input);
    }

    void Value::set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const
    {
        const mx::OutputPtr output = mtlx_utils::add_or_get_output(node_graph, type_, output_name);
        set_as_node_graph_output(output);
    }

    void Value::set_as_node_graph_input(const mx::NodeGraphPtr& node_graph, const string& input_name) const
    {
        const mx::InputPtr input = mtlx_utils::add_or_get_input(node_graph, type_, input_name);
        set_as_node_graph_input(input);
    }

    void Value::set_as_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name) const
    {
        const mx::InputPtr input = mtlx_utils::add_or_get_input(node_def, type_, input_name);
        set_as_node_def_input(input);
    }
}
