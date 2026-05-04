//
// Created by jaket on 25/03/2026.
//

#ifndef MXSLC_DOTEXPRESSION_H
#define MXSLC_DOTEXPRESSION_H

#include "Expression.h"

class DotAccessor;

class DotExpression final : public Expression
{
public:
    DotExpression(ExprPtr expr, Token property);

    ExprPtr instantiate_template_types(const TypePtr& template_type) const override;

protected:
    void init_subexpressions(const vector<TypePtr>& types) override;
    void init_impl(const vector<TypePtr>& types) override;
    TypePtr type_impl() const override;
    VarPtr evaluate_impl() const override;

private:
    ExprPtr expr_;
    shared_ptr<DotAccessor> accessor_;
};

#endif //MXSLC_DOTEXPRESSION_H
