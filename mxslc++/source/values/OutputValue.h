//
// Created by jaket on 10/01/2026.
//

#ifndef MXSLC_OUTPUTVALUE_H
#define MXSLC_OUTPUTVALUE_H

#include <MaterialXCore/Node.h>
#include "Value.h"
#include "runtime/TypeInfo.h"
#include "utils/common.h"

class OutputValue final : public Value
{
public:
    explicit OutputValue(mx::OutputPtr output) : output_{std::move(output)}
    {
        type_ = std::make_shared<TypeInfo>(output_->getType());
    }

    void set_as_node_input(const mx::NodePtr& node, const string& input_name) const override
    {
        node->setConnectedOutput(input_name, output_);
    }

    void set_as_node_graph_output(const mx::GraphElementPtr& node_graph, const string& output_name) const override
    {
        const mx::OutputPtr output = node_graph->addOutput(output_name, output_->getType());
        output->setConnectedOutput(output_);
    }

    [[nodiscard]] TypeInfoPtr type() const override { return type_; }
    [[nodiscard]] string str() const override { return output_->asString(); }

private:
    mx::OutputPtr output_;
    TypeInfoPtr type_;
};

#endif //MXSLC_OUTPUTVALUE_H
