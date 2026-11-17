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
        explicit DefaultGeomPropValue(mx::GeomPropDefPtr geomprop);

        bool equals(const ValuePtr& other) const override;

        void set_as_node_input(const mx::InputPtr& input) const override;
        void set_as_node_graph_input(const mx::InputPtr& input) const override;
        void set_as_node_graph_output(const mx::OutputPtr& output) const override;
        void set_as_node_def_input(const mx::InputPtr& input) const override;

        string to_string() const override;

    private:
        mx::GeomPropDefPtr geomprop_;
    };
}

#endif //MXSLC_DEFAULTGEOMPROPVALUE_H
