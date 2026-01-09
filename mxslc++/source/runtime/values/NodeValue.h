//
// Created by jaket on 01/01/2026.
//

#ifndef FENNEC_NODEVALUE_H
#define FENNEC_NODEVALUE_H

#include <MaterialXCore/Node.h>
#include "utils/common.h"
#include "Value.h"
#include "runtime/Type.h"
#include "mtlx/mtlx_utils.h"

class NodeValue final : public Value
{
public:
    explicit NodeValue(const mx::NodePtr& node) : node_{node}, type_{node_->getType()} { }

    void set_as_node_input(const mx::NodePtr& node, const string& input_name) const override
    {
        node->setConnectedNode(input_name, node_);
    }

    void set_as_node_graph_output(const mx::GraphElementPtr& node_graph, const string& output_name) const override
    {
        const mx::OutputPtr output = node_graph->addOutput(output_name, type_.str());
        output->setConnectedNode(node_);
    }

    void set_as_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name) const override
    {

    }

    [[nodiscard]] const Type& type() const override { return type_; }
    [[nodiscard]] string str() const override { return as_string(node_); }

private:
    mx::NodePtr node_;
    Type type_;
};

#endif //FENNEC_NODEVALUE_H
