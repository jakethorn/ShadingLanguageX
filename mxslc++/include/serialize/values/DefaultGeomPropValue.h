//
// Created by jaket on 15/09/2026.
//

#ifndef MXSLC_DEFAULTGEOMPROPVALUE_H
#define MXSLC_DEFAULTGEOMPROPVALUE_H

#include "serialize/values/Value.h"
#include "Primitive.h"

namespace mxslc::serialize::values
{
    class DefaultGeomPropValue final : public Value
    {
    public:
        explicit DefaultGeomPropValue(string name);

        bool equals(const ValuePtr& other) const override;

        void set_as_node_input(const mx::InputPtr& input) const override;
        void set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const override;
        void set_as_node_graph_input(const mx::NodeGraphPtr& node_graph, const string& input_name) const override;
        void set_as_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name) const override;

        string to_string() const override;

    private:
        static TypePtr get_type(const string& name);

        string name_;
    };
}

#endif //MXSLC_DEFAULTGEOMPROPVALUE_H
