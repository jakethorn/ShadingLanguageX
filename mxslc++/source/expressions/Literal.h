//
// Created by jaket on 27/11/2025.
//

#ifndef FENNEC_LITERAL_H
#define FENNEC_LITERAL_H

#include "Expression.h"

class Literal final : public Expression
{
public:
    Literal(const Runtime& runtime, basic_t value) : Expression{runtime}, value_{std::move(value)}, type_{value_} { }
    Literal(const Runtime& runtime, Token token) : Expression{runtime, std::move(token)}, value_{token_.literal()}, type_{value_} { }

    [[nodiscard]] ExprPtr instantiate_template_types(const Type& template_type) const override;

    [[nodiscard]] const basic_t& value() const { return value_; }

protected:
    [[nodiscard]] const Type& type_impl() const override;
    [[nodiscard]] ValuePtr evaluate_impl() const override;

private:
    basic_t value_;
    Type type_;
};

#endif //FENNEC_LITERAL_H
