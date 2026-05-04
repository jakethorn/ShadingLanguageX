//
// Created by jaket on 26/03/2026.
//

#include "Value.h"

#include <cassert>

#include "mtlx/mtlx_utils.h"
#include "runtime/Type.h"

Value::Value(TypePtr type) : type_{std::move(type)}
{
    assert(type_->is_primitive());
}

void Value::set_as_node_input(const mx::NodePtr& node, const string& input_name) const
{
    const mx::InputPtr input = add_or_get_input(node, type_->name(), input_name);
    set_as_node_input(input);
}
