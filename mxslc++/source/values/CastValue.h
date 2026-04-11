//
// Created by jaket on 27/03/2026.
//

#ifndef MXSLC_CASTVALUE_H
#define MXSLC_CASTVALUE_H

#include "Value.h"

class CastValue final : public Value
{
public:
    CastValue(ValuePtr value, TypeInfoPtr type) : Value{std::move(type)}, value_{std::move(value)} { }

    ValuePtr subvalue(const size_t i) const override { return value_->subvalue(i); }
    void set_subvalue(const size_t i, const ValuePtr& value) override { value_->set_subvalue(i, value); }

    void set_as_node_input(const mx::NodePtr& node, const string& input_name) const override
    {
        value_->set_as_node_input(node, input_name);
    }

    void set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const override
    {
        value_->set_as_node_graph_output(node_graph, output_name);
    }

    void set_as_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name) const override
    {
        value_->set_as_node_def_input(node_def, input_name);
    }

    void set_name(const string& name) override
    {
        value_->set_name(name);
    }

    string str() const override
    {
        return value_->str();
    }

private:
    ValuePtr value_;
};

#endif //MXSLC_CASTVALUE_H
