//
// Created by jaket on 07/01/2026.
//

#ifndef MXSLC_INTERFACEVALUE_H
#define MXSLC_INTERFACEVALUE_H

#include "utils/common.h"
#include "Value.h"
#include "runtime/TypeInfo.h"

class InterfaceValue final : public Value
{
public:
    InterfaceValue(string name, TypeInfoPtr type) : name_{std::move(name)}, type_{std::move(type)} { }

    void set_as_node_input(const mx::NodePtr& node, const string& input_name) const override
    {
        const mx::InputPtr input = node->addInput(input_name, type_->name());
        input->setInterfaceName(name_);
    }

    void set_as_node_graph_output(const mx::GraphElementPtr& node_graph, const string& output_name) const override
    {
        const mx::OutputPtr output = node_graph->addOutput(output_name, type_->name());
        output->setInterfaceName(name_);
    }

    [[nodiscard]] TypeInfoPtr type() const override { return type_; }
    [[nodiscard]] string str() const override { return name_; }

private:
    string name_;
    TypeInfoPtr type_;
};

#endif //MXSLC_INTERFACEVALUE_H
