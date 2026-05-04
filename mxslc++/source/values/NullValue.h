//
// Created by jaket on 31/12/2025.
//

#ifndef FENNEC_NULLVALUE_H
#define FENNEC_NULLVALUE_H

#include "Value.h"

class NullValue final : public Value
{
public:
    explicit NullValue(TypePtr type) : Value{std::move(type)} { }

    void set_as_node_input(const mx::InputPtr& input) const override
    {
        input->getParent()->removeChild(input->getName());
    }

    void set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const override
    {
        throw CompileError{"Cannot return null value"s};
    }

    string str() const override { return "null"s; }
};

#endif //FENNEC_NULLVALUE_H
