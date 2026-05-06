//
// Created by jaket on 06/05/2026.
//

#ifndef MXSLC_THISEXPRESSION_H
#define MXSLC_THISEXPRESSION_H

#include "Expression.h"

class ThisExpression final : public Expression
{
public:
    ThisExpression() : Expression{} { }
    explicit ThisExpression(Token token) : Expression{std::move(token)} { }

    ExprPtr instantiate_template_types(const TypePtr& template_type) const override;

protected:
    void init_impl(const vector<TypePtr>& types) override;
    TypePtr type_impl() const override;
    VarPtr evaluate_impl() const override;

private:
    VarPtr this_;
};

#endif //MXSLC_THISEXPRESSION_H
