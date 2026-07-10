//
// Created by jaket on 10/01/2026.
//

#ifndef MXSLC_NODEOUTPUTVALUE_H
#define MXSLC_NODEOUTPUTVALUE_H

#include "serialize/values/Value.h"

namespace mxslc::values
{
    class NodeOutputValue final : public Value
    {
    public:
        NodeOutputValue(TypePtr type, mx::NodePtr node, string output_name);

        mx::NodePtr node() const { return node_; }
        void set_node_name(const string& name) const;

        bool equals(const ValuePtr& other) const override;

        void set_as_node_input(const mx::InputPtr& input) const override;
        void set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const override;

        string str() const override;

    private:
        mx::NodePtr node_;
        string output_name_;

        mutable bool is_node_name_set_{false};
    };
}

#endif //MXSLC_NODEOUTPUTVALUE_H
