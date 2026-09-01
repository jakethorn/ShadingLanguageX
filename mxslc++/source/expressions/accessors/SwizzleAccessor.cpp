//
// Created by jaket on 01/08/2026.
//

#include "expressions/accessors/SwizzleAccessor.h"

#include "runtime/variables/SwizzleVariable.h"
#include "utils/swizzle_utils.h"

namespace mxslc::expressions
{
    SwizzleAccessor::SwizzleAccessor(ExprPtr value_expr, string swizzle)
        : value_expr_{std::move(value_expr)}, swizzle_{std::move(swizzle)}
    {

    }

    TypePtr SwizzleAccessor::type() const
    {
        return swizzle_utils::get_swizzle_type(swizzle_);
    }

    VarPtr SwizzleAccessor::evaluate() const
    {
        return std::make_shared<SwizzleVariable>(value_expr_, swizzle_);
    }
}
