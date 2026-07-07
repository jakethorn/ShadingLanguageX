//
// Created by jaket on 28/05/2026.
//

#ifndef MXSLC_COMPONENTACCESSOR_H
#define MXSLC_COMPONENTACCESSOR_H

#include "expressions/accessors/Accessor.h"
#include "utils/common.h"

class ComponentAccessor final : public Accessor
{
public:
    ComponentAccessor(ExprPtr value_expr, ExprPtr index_expr);

    TypePtr type() const override;
    VarPtr evaluate() const override;

private:
    ExprPtr func_call_;
};

#endif //MXSLC_COMPONENTACCESSOR_H
