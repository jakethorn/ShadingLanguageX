//
// Created by jaket on 31/12/2025.
//

#ifndef FENNEC_NULLEXPRESSION_H
#define FENNEC_NULLEXPRESSION_H

#include "Expression.h"

class NullExpression final : public Expression
{
public:
    explicit NullExpression(const Runtime& runtime) : Expression{runtime} { }
    NullExpression(const Runtime& runtime, Token token) : Expression{runtime, std::move(token)} { }

    [[nodiscard]] ExprPtr instantiate_template_types(const TypeInfoPtr& template_type) const override;

protected:
    void init_impl(const vector<TypeInfoPtr>& types) override;
    [[nodiscard]] TypeInfoPtr type_impl() const override;
    [[nodiscard]] ValuePtr evaluate_impl() const override;

private:
    TypeInfoPtr type_;
};

#endif //FENNEC_NULLEXPRESSION_H
