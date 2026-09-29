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
        IncrementOperator(ExprPtr value_expr, Token op, bool prefix);

        const ExprPtr& value_expression() const { return value_expr_; }
        bool is_prefix() const { return prefix_; }

        ExprPtr monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;
        Precedence precedence() const override;

    protected:
        void init_subexpressions(const vector<TypePtr>& types) override;
        TypePtr type_impl() const override;
        VarPtr evaluate_impl() const override;

    private:
        ExprPtr value_expr_;
        bool prefix_;
        bool increment_;
    };
}

#endif //MXSLC_INCREMENTOPERATOR_H
