//
// Created by jaket on 27/10/2026.
//

#ifndef MXSLC_RUNTIMEFIELDACCESSOR_H
#define MXSLC_RUNTIMEFIELDACCESSOR_H

#include "expressions/accessors/Accessor.h"
#include "common.h"

namespace mxslc::expressions
{
    class RuntimeFieldAccessor final : public Accessor
    {
    public:
        RuntimeFieldAccessor(ExprPtr value_expr, VarPtr index);

        TypePtr type() const override;
        VarPtr evaluate() const override;

    private:
        ExprPtr value_expr_;
        VarPtr index_;
    };
}

#endif //MXSLC_RUNTIMEFIELDACCESSOR_H
