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
    NodeValue(mx::NodePtr node, TypeInfoPtr type) : Value{std::move(type)}, node_{std::move(node)}
    {
        subvalues_.resize(type_->field_count());
    }

    [[nodiscard]] size_t subvalue_count() const override
    {
        return node_->isMultiOutputType() ? node_->getOutputCount() : 0;
    }

    [[nodiscard]] ValuePtr subvalue(const size_t i) const override
    {
        if (i < subvalues_.size() and subvalues_[i] != nullptr)
            return subvalues_[i];

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

    void set_subvalue(const size_t i, const ValuePtr& value) override
    {
        subvalues_[i] = value;
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
        node_->setName(name);
    }

    [[nodiscard]] ValuePtr cast_impl(const TypeInfoPtr& type) const override
    {
        return std::make_shared<NodeValue>(node_, type);
    }

    [[nodiscard]] string str() const override { return as_string(node_); }

private:
    mx::NodePtr node_;
    vector<ValuePtr> subvalues_;
};

#endif //FENNEC_NODEVALUE_H
