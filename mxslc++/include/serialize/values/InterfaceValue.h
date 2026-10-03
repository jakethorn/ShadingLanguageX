//
// Created by jaket on 07/01/2026.
//

#ifndef MXSLC_INTERFACEVALUE_H
#define MXSLC_INTERFACEVALUE_H

#include "serialize/values/Value.h"

namespace mxslc::serialize::values
{
    class InterfaceValue final : public Value
    {
    public:
        InterfaceValue(TypePtr type, string interface_name);

        bool equals(const ValuePtr& other) const override;

        void set_as_node_input(const mx::InputPtr& input) const override;
        void set_as_node_graph_input(const mx::InputPtr& input) const override;
        void set_as_node_graph_output(const mx::OutputPtr& output) const override;

        string to_string() const override;

    private:
        string interface_name_;
    };
}

#endif //MXSLC_INTERFACEVALUE_H
