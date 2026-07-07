//
// Created by jaket on 25/03/2026.
//

#ifndef MXSLC_DOTEXPRESSION_H
#define MXSLC_DOTEXPRESSION_H

#include "expressions/Expression.h"
#include "expressions/accessors/Accessor.h"

class DotOperator final : public Expression
{
public:
    DotOperator(ExprPtr expr, Token property);

    ExprPtr instantiate_template_types(const TypePtr& template_type) const override;

protected:
    void init_subexpressions(const vector<TypePtr>& types) override;
    void init_impl(const vector<TypePtr>& types) override;
    TypePtr type_impl() const override;
    VarPtr evaluate_impl() const override;

private:
    ExprPtr expr_;
    AccessorPtr accessor_;
};

#endif //MXSLC_DOTEXPRESSION_H
