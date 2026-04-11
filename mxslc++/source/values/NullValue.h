//
// Created by jaket on 31/12/2025.
//

#ifndef FENNEC_NULLVALUE_H
#define FENNEC_NULLVALUE_H

#include "Value.h"
#include "runtime/TypeInfo.h"

class NullValue final : public Value
{
public:
    explicit NullValue(TypeInfoPtr type) : Value{std::move(type)} { }

    void set_as_node_input(const mx::NodePtr& node, const string& input_name) const override
    {
        node->removeInput(input_name);
    }

    void set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const override
    {
        throw CompileError{"Cannot return null value"s};
    }

    string str() const override { return "null"s; }
};

#endif //FENNEC_NULLVALUE_H
