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

    VarPtr variable() const override;

    ExprPtr instantiate_template_types(const TypeInfoPtr& template_type) const override;

protected:
    void init_subexpressions(const vector<TypeInfoPtr>& types) override;
    TypeInfoPtr type_impl() const override;
    ValuePtr evaluate_impl() const override;
    void assign_impl(const ValuePtr &value) override;

private:
    const string& property() const { return token_.lexeme(); }
    ExprPtr expr_;
};

#endif //MXSLC_DOTEXPRESSION_H
