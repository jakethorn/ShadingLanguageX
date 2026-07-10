//
// Created by jaket on 25/06/2026.
//

#ifndef MXSLC_NODEGRAPHVALUE_H
#define MXSLC_NODEGRAPHVALUE_H

#include "serialize/values/Value.h"

namespace mxslc::values
{
    class NodeGraphValue final : public Value
    {
    public:
        explicit NodeGraphValue(const mx::NodeGraphPtr& node_graph);
        NodeGraphValue(TypePtr type, string node_graph_name);

        bool equals(const ValuePtr& other) const override;

        void set_as_node_input(const mx::InputPtr& input) const override;
        void set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const override;

        string str() const override;

    private:
        string name_;
    };
}

#endif //MXSLC_NODEGRAPHVALUE_H
