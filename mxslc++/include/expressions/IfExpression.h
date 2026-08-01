//
// Created by jaket on 01/08/2026.
//

#ifndef MXSLC_IFEXPRESSION_H
#define MXSLC_IFEXPRESSION_H

#include "expressions/Expression.h"

namespace mxslc::expressions
{
    class IfExpression final : public Expression
    {
    public:
        IfExpression(ExprPtr cond_expr, ExprPtr then_expr, ExprPtr else_expr, Token token = {});

        ExprPtr monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;

    protected:
        void init_subexpressions(const vector<TypePtr>& types) override;
        void init_impl(const vector<TypePtr>& types) override;
        TypePtr type_impl() const override;
        VarPtr evaluate_impl() const override;

    private:
        ExprPtr cond_expr_;
        ExprPtr then_expr_;
        ExprPtr else_expr_;
    };
}
#endif //MXSLC_IFEXPRESSION_H
