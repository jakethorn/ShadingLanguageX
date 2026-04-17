//
// Created by jaket on 14/04/2026.
//

#include "InterfaceValue.h"

#include "mtlx/mtlx_utils.h"
#include "runtime/TypeInfo.h"

InterfaceValue::InterfaceValue(TypeInfoPtr type, string name)
    : Value{std::move(type)}, name_{std::move(name)}
{

}

void InterfaceValue::set_as_node_input(const mx::NodePtr& node, const string& input_name) const
{
    const mx::InputPtr input = node->addInput(input_name, type_->name());
    set_interface(input, name_);
}

void InterfaceValue::set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const
{
    const mx::OutputPtr output = add_or_get_output(node_graph, type_, output_name);
    set_interface(output, name_);
}

string InterfaceValue::str() const
{
    return name_;
}
