//
// Created by jaket on 21/05/2026.
//

#ifndef MXSLC_IFEXPRESSION_H
#define MXSLC_IFEXPRESSION_H

#include "FunctionCall.h"

class IfExpression final : public FunctionCall
{
public:
    IfExpression(ExprPtr cond_expr, ExprPtr then_expr, ExprPtr else_expr, Token token);

    ExprPtr instantiate_template_types(const TypePtr& template_type) const override;

protected:
    void init_subexpressions(const vector<TypePtr>& types) override;
    void init_impl(const vector<TypePtr>& types) override;
    TypePtr type_impl() const override;
    VarPtr evaluate_impl() const override;

    ExprPtr cond_expr_;
    ExprPtr then_expr_;
    ExprPtr else_expr_;
};

#endif //MXSLC_IFEXPRESSION_H
