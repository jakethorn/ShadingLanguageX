//
// Created by jaket on 28/05/2026.
//

#ifndef MXSLC_RUNTIMEEXPRESSION_H
#define MXSLC_RUNTIMEEXPRESSION_H

#include "Expression.h"

class RuntimeExpression final : public Expression
{
public:
    explicit RuntimeExpression(VarPtr value);

    ExprPtr instantiate_template_types(const TypePtr& template_type) const override;

protected:
    TypePtr type_impl() const override;
    VarPtr evaluate_impl() const override;

private:
    VarPtr value_;
};

#endif //MXSLC_RUNTIMEEXPRESSION_H
