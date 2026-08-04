//
// Created by jaket on 03/05/2026.
//

#ifndef MXSLC_PORTVARIABLE_H
#define MXSLC_PORTVARIABLE_H

#include <MaterialXCore/Interface.h>

#include "common.h"
#include "Variable.h"

namespace mxslc::runtime
{
    /// Variable representing the input of a node
    class PortVariable final : public Variable
    {
    public:
        explicit PortVariable(mx::InputPtr input);

    protected:
        ValuePtr value_impl() const override;
        void copy_value_impl(ValuePtr value) override;
        void set_node_name(const string& name) const override;

    private:
        mx::InputPtr input_;
    };
}

#endif //MXSLC_PORTVARIABLE_H
