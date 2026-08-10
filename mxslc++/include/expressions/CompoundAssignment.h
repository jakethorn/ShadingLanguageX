//
// Created by jaket on 15/05/2026.
//

#ifndef MXSLC_COMPOUNDASSIGNMENT_H
#define MXSLC_COMPOUNDASSIGNMENT_H

#include "expressions/Expression.h"

namespace mxslc::expressions
{
    class CompoundAssignment final : public Expression
    {
    public:
        CompoundAssignment(ExprPtr lhs_expr, Token op, ExprPtr rhs_expr);

        ExprPtr monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;

    protected:
        void init_impl(const vector<TypePtr>& types) override;
        TypePtr type_impl() const override;
        VarPtr evaluate_impl() const override;

    private:
        ExprPtr lhs_expr_;
        ExprPtr rhs_expr_;

        ExprPtr func_call_;
    };
}

#endif //MXSLC_COMPOUNDASSIGNMENT_H
