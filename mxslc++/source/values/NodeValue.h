//
// Created by jaket on 01/01/2026.
//

#ifndef FENNEC_NODEVALUE_H
#define FENNEC_NODEVALUE_H

#include <MaterialXCore/Node.h>

#include "OutputValue.h"
#include "utils/common.h"
#include "Value.h"
#include "mtlx/mtlx_utils.h"

class NodeValue final : public Value
{
public:
    NodeValue(mx::NodePtr node, TypeInfoPtr type) : Value{std::move(type)}, node_{std::move(node)} { }

    [[nodiscard]] size_t subvalue_count() const override
    {
        return node_->isMultiOutputType() ? node_->getOutputCount() : 0;
    }

    [[nodiscard]] ValuePtr subvalue(const size_t i) const override
    {
        if (node_->isMultiOutputType())
        {
            size_t j = 0;
            for (const mx::OutputPtr& output : node_->getOutputs())
            {
                if (i == j++)
                    return std::make_shared<OutputValue>(output, type_->field_type(i));
            }
        }

        throw std::out_of_range{"Subvalue index out of range"};
    }

    void set_as_node_input(const mx::NodePtr& node, const string& input_name) const override
    {
        node->setConnectedNode(input_name, node_);
    }

    void set_as_node_graph_output(const mx::GraphElementPtr& node_graph, const string& output_name) const override
    {
        const mx::OutputPtr output = node_graph->addOutput(output_name, node_->getType());
        output->setConnectedNode(node_);
    }

    void set_name(const string& name) override
    {
        const string valid_name = node_->getParent()->createValidChildName(name);
        node_->setName(valid_name);
    }

    [[nodiscard]] string str() const override { return ::as_string(node_); }

private:
    mx::NodePtr node_;
};

#endif //FENNEC_NODEVALUE_H
