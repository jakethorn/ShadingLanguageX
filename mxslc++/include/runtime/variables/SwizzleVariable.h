//
// Created by jaket on 02/08/2026.
//

#ifndef MXSLC_SWIZZLEVARIABLE_H
#define MXSLC_SWIZZLEVARIABLE_H

#include "common.h"
#include "Variable.h"

namespace mxslc::runtime
{
    /// Variable representing channel access of a vector/color, e.g., vec.xyz
    class SwizzleVariable final : public Variable
    {
    public:
        explicit SwizzleVariable(ExprPtr value_expr, string swizzle);

        bool is_temporary() const override { return false; }
        bool is_local() override { return true; }

    protected:
        ValuePtr value_impl() const override;
        void copy_value_impl(ValuePtr value) override;
        void set_node_name(const string& name) const override;

    private:
        ExprPtr value_expr_;
        string swizzle_;

        mutable ValuePtr swizzle_value_;
    };
}

#endif //MXSLC_SWIZZLEVARIABLE_H
