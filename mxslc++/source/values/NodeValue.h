//
// Created by jaket on 01/01/2026.
//

#ifndef FENNEC_NODEVALUE_H
#define FENNEC_NODEVALUE_H

#include "utils/common.h"
#include "Value.h"

class NodeValue final : public Value
{
public:
    NodeValue(mx::NodePtr node, TypeInfoPtr type);

    void set_as_node_input(const mx::NodePtr& node, const string& input_name) const override;
    void set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const override;

    void set_node_name(const string& name) const;

    string str() const override;

private:
    mx::NodePtr node_;
};

#endif //FENNEC_NODEVALUE_H
