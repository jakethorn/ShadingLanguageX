//
// Created by jaket on 28/05/2026.
//

#ifndef MXSLC_COMPONENTACCESSOR_H
#define MXSLC_COMPONENTACCESSOR_H

#include "expressions/accessors/Accessor.h"
#include "common.h"

namespace mxslc::expressions
{
    class ComponentAccessor final : public Accessor
    {
    public:
        ComponentAccessor(const ExprPtr& value_expr, const ExprPtr& index_expr);

        TypePtr type() const override;
        VarPtr evaluate() const override;

    private:
        VarPtr component_var_;
    };
}

#endif //MXSLC_COMPONENTACCESSOR_H
