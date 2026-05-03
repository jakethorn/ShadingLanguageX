//
// Created by jaket on 10/01/2026.
//

#ifndef MXSLC_OUTPUTVALUE_H
#define MXSLC_OUTPUTVALUE_H

#include "Value.h"
#include "utils/common.h"

class OutputValue final : public Value
{
public:
    OutputValue(mx::OutputPtr output, TypePtr type);
    OutputValue(const mx::NodePtr& node, const string& output_name, TypePtr type);

    string str() const override;

protected:
    void set_as_node_input(const mx::NodePtr& node, const string& input_name) const override;
    void set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const override;

private:
    mx::OutputPtr output_;
};

#endif //MXSLC_OUTPUTVALUE_H
