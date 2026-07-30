//
// Created by jaket on 31/12/2025.
//

#ifndef FENNEC_NULLVALUE_H
#define FENNEC_NULLVALUE_H

#include "serialize/values/Value.h"
#include "utils/mtlx_utils.h"

namespace mxslc::serialize::values
{
    class NullValue final : public Value
    {
    public:
        explicit NullValue(TypePtr type) : Value{std::move(type)} { }

        bool equals(const ValuePtr& other) const override
        {
            return cast_value<NullValue>(other) != nullptr;
        }

        void set_as_node_input(const mx::InputPtr& input) const override
        {
            mtlx_utils::remove_port(input);
        }

        void set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const override
        {
            node_graph->removeChild(output_name);
        }

        string to_string() const override { return "null"; }
    };
}

#endif //FENNEC_NULLVALUE_H
