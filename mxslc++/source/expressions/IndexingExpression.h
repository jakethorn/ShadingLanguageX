//
// Created by jaket on 11/01/2026.
//

#ifndef MXSLC_INDEXINGEXPRESSION_H
#define MXSLC_INDEXINGEXPRESSION_H

#include "Expression.h"

class IndexingExpression final : public Expression
{
public:
    IndexingExpression(const Runtime& runtime, ExprPtr expr, ExprPtr index)
        : Expression{runtime, index->token()}, expr_{std::move(expr)}, index_expr_{std::move(index)} { }

    VarPtr variable() const override;

    ExprPtr instantiate_template_types(const TypeInfoPtr& template_type) const override;

    void assign_impl(const ValuePtr &value) override;

protected:
    void init_subexpressions(const vector<TypeInfoPtr>& types) override;
    void init_impl(const vector<TypeInfoPtr>& types) override;
    TypeInfoPtr type_impl() const override;
    ValuePtr evaluate_impl() const override;

private:
    ExprPtr expr_;
    ExprPtr index_expr_;
    size_t index_ = 0;
};

#endif //MXSLC_INDEXINGEXPRESSION_H
