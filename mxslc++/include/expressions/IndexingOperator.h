//
// Created by jaket on 11/01/2026.
//

#ifndef MXSLC_INDEXINGEXPRESSION_H
#define MXSLC_INDEXINGEXPRESSION_H

#include "expressions/Expression.h"
#include "expressions/accessors/Accessor.h"

class IndexingOperator final : public Expression
{
public:
    IndexingOperator(ExprPtr value_expr, ExprPtr index_expr)
        : Expression{index_expr->token()}, value_expr_{std::move(value_expr)}, index_expr_{std::move(index_expr)} { }

    ExprPtr instantiate_template_types(const TypePtr& template_type) const override;

protected:
    void init_subexpressions(const vector<TypePtr>& types) override;
    void init_impl(const vector<TypePtr>& types) override;
    TypePtr type_impl() const override;
    VarPtr evaluate_impl() const override;

private:
    ExprPtr value_expr_;
    ExprPtr index_expr_;

    AccessorPtr accessor_;
};

#endif //MXSLC_INDEXINGEXPRESSION_H
