//
// Created by jaket on 31/12/2025.
//

#ifndef FENNEC_NULLEXPRESSION_H
#define FENNEC_NULLEXPRESSION_H

#include "Expression.h"

class NullExpression final : public Expression
{
public:
    NullExpression(const Runtime& runtime) : Expression{runtime} { }
    NullExpression(const Runtime& runtime, Token token) : Expression{runtime, std::move(token)} { }

    [[nodiscard]] ExprPtr instantiate_templated_types(const Type& template_type) const override;

protected:
    void init_impl(const vector<Type>& types) override;
    [[nodiscard]] const Type& type_impl() const override;
    [[nodiscard]] ValuePtr evaluate_impl() const override;

private:
    Type type_;
};

#endif //FENNEC_NULLEXPRESSION_H
