//
// Created by jaket on 11/09/2026.
//

#ifndef MXSLC_STRINGVALUE_H
#define MXSLC_STRINGVALUE_H

#include "serialize/values/Value.h"
#include "Primitive.h"

namespace mxslc::serialize::values
{
    class StringValue final : public Value
    {
    public:
        StringValue(string value, TypePtr type);

        const string& get() const { return value_; }

        bool equals(const ValuePtr& other) const override;

        void set_as_node_input(const mx::InputPtr& input) const override;
        void set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const override;
        void set_as_node_graph_input(const mx::NodeGraphPtr& node_graph, const string& input_name) const override;
        void set_as_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name) const override;

        string to_string() const override;

    private:
        string value_;
    };
}

#endif //MXSLC_STRINGVALUE_H
