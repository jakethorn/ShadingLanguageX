//
// Created by jaket on 02/08/2026.
//

#include "runtime/variables/SwizzleVariable.h"
#include "runtime/ArgumentList.h"
#include "expressions/RuntimeExpression.h"
#include "runtime/interface.h"
#include "runtime/utils/invoke.h"
#include "utils/swizzle_utils.h"

namespace mxslc::runtime
{
    SwizzleVariable::SwizzleVariable(ExprPtr value_expr, string swizzle)
        : Variable{TokenType::Mutable, swizzle_utils::get_swizzle_type(swizzle)}, value_expr_{std::move(value_expr)}, swizzle_{std::move(swizzle)}
    {

    }

    ValuePtr SwizzleVariable::value_impl() const
    {
        if (swizzle_value_ == nullptr)
            swizzle_value_ = swizzle_utils::evaluate_swizzle(value_expr_, swizzle_)->value();
        return swizzle_value_;
    }

    void SwizzleVariable::copy_value_impl(ValuePtr value)
    {
        if (swizzle_utils::has_duplicate_channels(swizzle_))
            throw CompileError{"Swizzle cannot have duplicate channels during assignment"};

        // lhs.swizzle = rhs
        // TODO this separate is useless if all child values are overridden anyway
        // TODO (cont.) compare swizzle type and value_expr_ type, if they are the same, we dont need to separate
        const VarPtr lhs_values = runtime_utils::invoke_function("separate", value_expr_);
        const VarPtr rhs_values = runtime_utils::invoke_function("separate", value);

        for (size_t i = 0; i < swizzle_.size(); ++i)
        {
            const int channel_index = swizzle_utils::get_channel_index(swizzle_[i]);
            lhs_values->child(channel_index)->uninitialize();
            lhs_values->child(channel_index)->copy(rhs_values->child(i));
        }

        const VarPtr values = runtime_utils::invoke_function(value_expr_->type(), "combine", lhs_values->children());
        value_expr_->assign(values);
    }

    void SwizzleVariable::set_node_name(const string& name) const
    {

    }
}
