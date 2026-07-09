//
// Created by jaket on 15/06/2026.
//

#ifndef MXSLC_RANGEEXPRESSION_H
#define MXSLC_RANGEEXPRESSION_H

#include "expressions/Expression.h"
#include "runtime/Variable.h"

namespace mxslc::expressions
{
    class RangeExpression final : public Expression
    {
    public:
        RangeExpression(ExprPtr lower_expr, ExprPtr upper_expr);
        RangeExpression(ExprPtr lower_expr, ExprPtr upper_expr, Token token);
        RangeExpression(ExprPtr lower_expr, ExprPtr step_expr, ExprPtr upper_expr);
        RangeExpression(ExprPtr lower_expr, ExprPtr step_expr, ExprPtr upper_expr, Token token);

        ExprPtr monomorphize(const TypePtr& template_type) const override;

    protected:
        void init_subexpressions(const vector<TypePtr>& types) override;
        void init_impl(const vector<TypePtr>& types) override;
        TypePtr type_impl() const override;
        VarPtr evaluate_impl() const override;

    private:
        template<typename T>
        void init_as()
        {
            T lower = lower_expr_->evaluate()->value_as<T>();
            T step = step_expr_ ? step_expr_->evaluate()->value_as<T>() : 1;
            T upper = upper_expr_->evaluate()->value_as<T>();

            vector<primitive_t> result;
            while (lower <= upper)
            {
                result.push_back(lower);
                lower += step;
            }

            range_ = Variable::create(result);
        }

        ExprPtr lower_expr_;
        ExprPtr step_expr_;
        ExprPtr upper_expr_;

        VarPtr range_;
    };
}

#endif //MXSLC_RANGEEXPRESSION_H
