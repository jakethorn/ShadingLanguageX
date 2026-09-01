//
// Created by jaket on 01/08/2026.
//

#ifndef MXSLC_SWIZZLEACCESSOR_H
#define MXSLC_SWIZZLEACCESSOR_H

#include "expressions/accessors/Accessor.h"

namespace mxslc::expressions
{
    class SwizzleAccessor final : public Accessor
    {
    public:
        SwizzleAccessor(ExprPtr value_expr, string swizzle);

        TypePtr type() const override;
        VarPtr evaluate() const override;

    private:
        ExprPtr value_expr_;
        string swizzle_;
    };
}

#endif //MXSLC_SWIZZLEACCESSOR_H
