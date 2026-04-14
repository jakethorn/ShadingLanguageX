//
// Created by jaket on 07/01/2026.
//

#ifndef MXSLC_INTERFACEVALUE_H
#define MXSLC_INTERFACEVALUE_H

#include "utils/common.h"
#include "Value.h"
#include "mtlx/mtlx_utils.h"
#include "runtime/TypeInfo.h"

class InterfaceValue final : public Value
{
public:
    InterfaceValue(string name, TypeInfoPtr type) : Value{std::move(type)}, name_{std::move(name)} { }

    void set_as_node_input(const mx::NodePtr& node, const string& input_name) const override
    {
        const mx::InputPtr input = node->addInput(input_name, type_->name());
        set_interface(input, name_);
    }

    void set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const override
    {
        const mx::OutputPtr output = add_or_get_output(node_graph, output_name, type_);
        set_interface(output, name_);
    }

    string str() const override { return name_; }

private:
    string name_;
};

#endif //MXSLC_INTERFACEVALUE_H
