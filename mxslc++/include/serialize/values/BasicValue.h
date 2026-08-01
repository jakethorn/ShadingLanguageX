//
// Created by jaket on 27/11/2025.
//

#ifndef FENNEC_BASICVALUE_H
#define FENNEC_BASICVALUE_H

#include "serialize/values/Value.h"
#include "Primitive.h"

namespace mxslc::serialize::values
{
    using runtime_utils::TypeName;

    class BasicValue final : public Value
    {
    public:
        explicit BasicValue(const mx::ValuePtr& value);
        explicit BasicValue(Primitive value);
        BasicValue(Primitive value, TypePtr type);

        const Primitive& get() const { return value_; }

        bool equals(const ValuePtr& other) const override;

        void set_as_node_input(const mx::InputPtr& input) const override;
        void set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const override;
        void set_as_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name) const override;

        string to_string() const override;

    private:
        string type_name() const;

        Primitive value_;
    };
}

#endif //FENNEC_BASICVALUE_H
