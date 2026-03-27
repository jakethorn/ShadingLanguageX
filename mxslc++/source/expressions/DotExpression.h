//
// Created by jaket on 25/03/2026.
//

#ifndef MXSLC_DOTEXPRESSION_H
#define MXSLC_DOTEXPRESSION_H

#include "Expression.h"

class DotExpression final : public Expression
{
public:
    DotExpression(const Runtime& runtime, ExprPtr expr, Token property)
        : Expression{runtime, std::move(property)}, expr_{std::move(expr)} { }

    [[nodiscard]] ExprPtr instantiate_template_types(const TypeInfoPtr& template_type) const override;

    void assign(const ValuePtr &value) override;

protected:
    void init_subexpressions(const vector<TypeInfoPtr>& types) override;
    [[nodiscard]] TypeInfoPtr type_impl() const override;
    [[nodiscard]] ValuePtr evaluate_impl() const override;

private:
    ExprPtr expr_;
    TypeInfoPtr type_;
};

#endif //MXSLC_DOTEXPRESSION_H
