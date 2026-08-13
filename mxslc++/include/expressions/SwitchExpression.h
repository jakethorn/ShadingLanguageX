//
// Created by jaket on 11/08/2026.
//

#ifndef MXSLC_SWITCHEXPRESSION_H
#define MXSLC_SWITCHEXPRESSION_H

#include "expressions/Expression.h"

namespace mxslc::expressions
{
    class SwitchExpression final : public Expression
    {
    public:
        SwitchExpression(ExprPtr index_expr, vector<ExprPtr> case_exprs, ExprPtr default_expr, Token token = {});

        ExprPtr monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;

    protected:
        void init_subexpressions(const vector<TypePtr>& types) override;
        void init_impl(const vector<TypePtr>& types) override;
        TypePtr type_impl() const override;
        VarPtr evaluate_impl() const override;

    private:
        ExprPtr index_expr_;
        vector<ExprPtr> case_exprs_;
        ExprPtr default_expr_;
    };
}

#endif //MXSLC_SWITCHEXPRESSION_H
