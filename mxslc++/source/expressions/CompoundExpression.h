//
// Created by jaket on 15/05/2026.
//

#ifndef MXSLC_COMPOUNDEXPRESSION_H
#define MXSLC_COMPOUNDEXPRESSION_H

#include "Expression.h"

class CompoundExpression final : public Expression
{
public:
    CompoundExpression(ExprPtr lhs_expr, Token op, ExprPtr rhs_expr);

    ExprPtr instantiate_template_types(const TypePtr& template_type) const override;

protected:
    void init_subexpressions(const vector<TypePtr>& types) override;
    TypePtr type_impl() const override;
    VarPtr evaluate_impl() const override;

private:
    ExprPtr lhs_expr_;
    ExprPtr rhs_expr_;

    ExprPtr func_call_;
};

#endif //MXSLC_COMPOUNDEXPRESSION_H
