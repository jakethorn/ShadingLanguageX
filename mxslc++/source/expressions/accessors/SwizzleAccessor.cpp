//
// Created by jaket on 01/08/2026.
//

#include "expressions/accessors/SwizzleAccessor.h"

#include "runtime/variables/SwizzleVariable.h"
#include "utils/swizzle_utils.h"

namespace mxslc::expressions
{
    SwizzleAccessor::SwizzleAccessor(VarPtr lhs, string swizzle) : swizzle_(swizzle)
    {
        swizzle_var_ = std::make_shared<SwizzleVariable>(std::move(lhs), std::move(swizzle));
    }

    TypePtr SwizzleAccessor::type() const
    {
        return swizzle_utils::get_swizzle_type(swizzle_);
    }

    VarPtr SwizzleAccessor::evaluate() const
    {
        VarPtr value = value_expr_->evaluate();
        return std::make_shared<SwizzleVariable>(value, swizzle_);
    }
}
