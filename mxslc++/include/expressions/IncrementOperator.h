//
// Created by jaket on 05/05/2026.
//

#ifndef MXSLC_INCREMENTOPERATOR_H
#define MXSLC_INCREMENTOPERATOR_H

#include "expressions/Expression.h"

namespace mxslc::expressions
{
    class IncrementOperator final : public Expression
    {
    public:
        IncrementOperator(ExprPtr lhs_expr, Token op, bool prefix);

        ExprPtr monomorphize(const TypePtr& template_type) const override;

    protected:
        void init_subexpressions(const vector<TypePtr>& types) override;
        TypePtr type_impl() const override;
        VarPtr evaluate_impl() const override;

    private:
        ExprPtr lhs_expr_;
        bool prefix_;
        bool increment_;
    };
}

#endif //MXSLC_INCREMENTOPERATOR_H
