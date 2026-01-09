//
// Created by jaket on 28/11/2025.
//

#ifndef FENNEC_IDENTIFIER_H
#define FENNEC_IDENTIFIER_H

#include <cassert>

#include "Expression.h"

class Variable;

class Identifier final : public Expression
{
public:
    Identifier(const Runtime& runtime, Token token) : Expression{runtime, std::move(token)} { }

    [[nodiscard]] ExprPtr instantiate_template_types(const Type& template_type) const override;

    [[nodiscard]] const string& name() const { return token_.lexeme(); }
    [[nodiscard]] const Variable& variable() const
    {
        assert(is_initialized_);
        return *var_;
    }

protected:
    void init_impl(const vector<Type> &types) override;
    [[nodiscard]] const Type& type_impl() const override;
    [[nodiscard]] ValuePtr evaluate_impl() const override;

private:
    const Variable* var_ = nullptr;
};

#endif //FENNEC_IDENTIFIER_H
