//
// Created by jaket on 07/01/2026.
//

#ifndef MXSLC_INTERFACEVALUE_H
#define MXSLC_INTERFACEVALUE_H

#include "utils/common.h"
#include "Value.h"
#include "runtime/Type.h"

class InterfaceValue final : public Value
{
public:
    InterfaceValue(string name, Type type) : name_{std::move(name)}, type_{std::move(type)} { }

    void set_as_node_input(const mx::NodePtr& node, const string& input_name) const override
    {
        const mx::InputPtr input = node->addInput(input_name, type_.str());
        input->setInterfaceName(name_);
    }

    void set_as_node_graph_output(const mx::GraphElementPtr& node_graph, const string& output_name) const override
    {
        const mx::OutputPtr output = node_graph->addOutput(output_name, type_.str());
        output->setInterfaceName(name_);
    }

    void set_as_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name) const override
    {

    }

    [[nodiscard]] const Type& type() const override { return type_; }
    [[nodiscard]] string str() const override { return name_; }

private:
    string name_;
    Type type_;
};

#endif //MXSLC_INTERFACEVALUE_H
