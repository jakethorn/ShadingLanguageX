//
// Created by jaket on 13/07/2026.
//

#ifndef MXSLC_CHANNELVARIABLE_H
#define MXSLC_CHANNELVARIABLE_H

#include "common.h"
#include "Variable.h"

namespace mxslc::runtime
{
    /// Variable representing channel access of a vector/color, e.g., vec[i]
    class ComponentVariable final : public Variable
    {
    public:
        explicit ComponentVariable(VarPtr value, VarPtr index);

        bool is_temporary() const override { return false; }
        bool is_local() override { return true; }

    protected:
        ValuePtr value_impl() const override;
        void copy_value_impl(ValuePtr value) override;
        void set_node_name(const string& name) const override;

    private:
        static TypePtr get_type(const VarPtr& value, const VarPtr& index);

        VarPtr value_;
        VarPtr index_;

        mutable ValuePtr component_value_;
    };
}

#endif //MXSLC_CHANNELVARIABLE_H
