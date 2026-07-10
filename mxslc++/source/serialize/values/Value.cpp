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

    void Value::set_as_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name) const
    {
        mx::InputPtr input = node_def->addInput(input_name, type_->name());
    }

    void Value::set_as_node_input(const mx::NodePtr& node, const string& input_name) const
    {
        const mx::InputPtr input = mtlx_utils::add_or_get_input(node, type_->name(), input_name);
        set_as_node_input(input);
    }
}
