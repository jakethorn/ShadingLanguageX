//
// Created by jaket on 25/06/2026.
//

#ifndef MXSLC_NODEGRAPHVALUE_H
#define MXSLC_NODEGRAPHVALUE_H

#include "utils/common.h"
#include "Value.h"

class NodeGraphValue final : public Value
{
public:
    explicit NodeGraphValue(mx::NodeGraphPtr node_graph);

    mx::NodeGraphPtr node_graph() const { return node_graph_; }

    bool equals(const ValuePtr& other) const override;

    void set_as_node_input(const mx::InputPtr& input) const override;
    void set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const override;

    string str() const override;

private:
    mx::NodeGraphPtr node_graph_;
};

#endif //MXSLC_NODEGRAPHVALUE_H
