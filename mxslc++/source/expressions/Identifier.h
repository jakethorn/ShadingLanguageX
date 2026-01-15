//
// Created by jaket on 28/11/2025.
//

#ifndef FENNEC_IDENTIFIER_H
#define FENNEC_IDENTIFIER_H

#include "Expression.h"
#include "runtime/Variable.h"

class Identifier final : public Expression
{
public:
    Identifier(const Runtime& runtime, Token token) : Expression{runtime, std::move(token)} { }

    [[nodiscard]] ExprPtr instantiate_template_types(const Type& template_type) const override;

protected:
    void init_impl(const vector<Type> &types) override;
    [[nodiscard]] const Type& type_impl() const override;
    [[nodiscard]] ValuePtr evaluate_impl() const override;

private:
    Variable var_;
};

#endif //FENNEC_IDENTIFIER_H
