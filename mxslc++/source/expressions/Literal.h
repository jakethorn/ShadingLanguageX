//
// Created by jaket on 27/11/2025.
//

#ifndef FENNEC_LITERAL_H
#define FENNEC_LITERAL_H

#include "Expression.h"

class Literal final : public Expression
{
public:
    Literal(const Runtime& runtime, Token token)
        : Expression{runtime, std::move(token)}, value_{token_.literal()} { }

    [[nodiscard]] ExprPtr instantiate_template_types(const TypeInfoPtr& template_type) const override;

protected:
    void init_impl(const vector<TypeInfoPtr>& types) override;
    [[nodiscard]] TypeInfoPtr type_impl() const override;
    [[nodiscard]] ValuePtr evaluate_impl() const override;

private:
    basic_t value_;
    TypeInfoPtr type_;
};

#endif //FENNEC_LITERAL_H
