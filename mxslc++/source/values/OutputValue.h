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
    OutputValue(mx::OutputPtr output, TypeInfoPtr type) : Value{std::move(type)}, output_{std::move(output)} { }

    void set_as_node_input(const mx::NodePtr& node, const string& input_name) const override
    {
        node->setConnectedOutput(input_name, output_);
    }

    void set_as_node_graph_output(const mx::GraphElementPtr& node_graph, const string& output_name) const override
    {
        const mx::OutputPtr output = node_graph->addOutput(output_name, output_->getType());
        output->setConnectedOutput(output_);
    }

    [[nodiscard]] ValuePtr cast_impl(const TypeInfoPtr& type) const override
    {
        return std::make_shared<OutputValue>(output_, type);
    }

    [[nodiscard]] string str() const override { return output_->asString(); }

private:
    mx::OutputPtr output_;
};

#endif //MXSLC_OUTPUTVALUE_H
