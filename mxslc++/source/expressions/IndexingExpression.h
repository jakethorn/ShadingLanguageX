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
        : Expression{runtime, index->token()}, expr_{std::move(expr)}, index_{std::move(index)} { }

    [[nodiscard]] ExprPtr instantiate_template_types(const TypeInfoPtr& template_type) const override;

protected:
    void init_subexpressions(const vector<TypeInfoPtr>& types) override;
    [[nodiscard]] TypeInfoPtr type_impl() const override;
    [[nodiscard]] ValuePtr evaluate_impl() const override;

private:
    ExprPtr expr_;
    ExprPtr index_;
    Type type_;
};

#endif //MXSLC_INDEXINGEXPRESSION_H
