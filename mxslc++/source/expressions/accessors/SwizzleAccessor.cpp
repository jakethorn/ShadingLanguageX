//
// Created by jaket on 01/08/2026.
//

#include "expressions/accessors/SwizzleAccessor.h"

#include "runtime/variables/SwizzleVariable.h"

namespace mxslc::expressions
{
    SwizzleAccessor::SwizzleAccessor(VarPtr lhs, string swizzle)
    {
        swizzle_var_ = std::make_shared<SwizzleVariable>(std::move(lhs), std::move(swizzle));
    }

    TypePtr SwizzleAccessor::type() const
    {
        return swizzle_var_->type();
    }

    VarPtr SwizzleAccessor::evaluate() const
    {
        return swizzle_var_;
    }
}
