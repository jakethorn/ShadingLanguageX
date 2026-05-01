//
// Created by jaket on 27/11/2025.
//

#ifndef FENNEC_LITERAL_H
#define FENNEC_LITERAL_H

#include "Expression.h"
#include "utils/str_utils.h"

class Literal final : public Expression
{
public:
    explicit Literal(Token token) : Expression{token}, value_{token.literal()} { }

    ExprPtr instantiate_template_types(const TypePtr& template_type) const override;

protected:
    void init_impl(const vector<TypePtr>& types) override;
    TypePtr type_impl() const override;
    VarPtr evaluate_impl() const override;

private:
    basic_t value_;
    TypePtr type_;
};

#endif //FENNEC_LITERAL_H
