//
// Created by jaket on 07/01/2026.
//

#ifndef MXSLC_INTERFACEVALUE_H
#define MXSLC_INTERFACEVALUE_H

#include "utils/common.h"
#include "Value.h"

class InterfaceValue final : public Value
{
public:
    InterfaceValue(TypePtr type, string name);

    bool equals(const ValuePtr& other) const override;

    void set_as_node_input(const mx::InputPtr& input) const override;
    void set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const override;

    string str() const override;

private:
    string name_;
};

#endif //MXSLC_INTERFACEVALUE_H
