//
// Created by jaket on 25/06/2026.
//

#ifndef MXSLC_NODEGRAPHOUTPUTVALUE_H
#define MXSLC_NODEGRAPHOUTPUTVALUE_H

#include "serialize/values/Value.h"

namespace mxslc::values
{
    class NodeGraphOutputValue final : public Value
    {
    public:
        NodeGraphOutputValue(TypePtr type, const mx::NodeGraphPtr& node_graph, string output_name);
        NodeGraphOutputValue(TypePtr type, string node_graph_name, string output_name);

        bool equals(const ValuePtr& other) const override;

        void set_as_node_input(const mx::InputPtr& input) const override;
        void set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const override;

        string str() const override;

    private:
        string node_graph_name_;
        string output_name_;
    };
}

#endif //MXSLC_NODEGRAPHOUTPUTVALUE_H
