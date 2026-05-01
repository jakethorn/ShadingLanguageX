//
// Created by jaket on 31/12/2025.
//

#ifndef FENNEC_NULLEXPRESSION_H
#define FENNEC_NULLEXPRESSION_H

#include "Expression.h"

class NullExpression final : public Expression
{
public:
    NullExpression() = default;
    explicit NullExpression(Token token) : Expression{std::move(token)} { }

    ExprPtr instantiate_template_types(const TypePtr& template_type) const override;

protected:
    void init_impl(const vector<TypePtr>& types) override;
    TypePtr type_impl() const override;
    VarPtr evaluate_impl() const override;

private:
    TypePtr type_;
};

#endif //FENNEC_NULLEXPRESSION_H
