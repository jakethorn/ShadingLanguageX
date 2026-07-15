//
// Created by jaket on 13/07/2026.
//

#ifndef MXSLC_COMPONENTVARIABLE_H
#define MXSLC_COMPONENTVARIABLE_H

#include "common.h"
#include "runtime/Variable.h"

namespace mxslc::runtime
{
    /// Variable representing component access of a vector/color, e.g., vec[i]
    class ComponentVariable final : public Variable
    {
    public:
        explicit ComponentVariable(ExprPtr value_expr, ExprPtr index_expr);

        bool is_temporary() const override { return false; }
        bool is_local() override { return true; }

    protected:
        ValuePtr value_impl() const override;
        void copy_value_impl(ValuePtr value) override;
        void set_node_name(const string& name) const override;

    private:
        static TypePtr get_type(const ExprPtr& value_expr, const ExprPtr& index_expr);

        ExprPtr value_expr_;
        ExprPtr index_expr_;

        mutable ValuePtr component_value_;
    };
}

#endif //MXSLC_COMPONENTVARIABLE_H
