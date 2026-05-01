//
// Created by jaket on 28/11/2025.
//

#ifndef FENNEC_IDENTIFIER_H
#define FENNEC_IDENTIFIER_H

#include "Expression.h"

class Identifier final : public Expression
{
public:
    explicit Identifier(Token name) : Expression{std::move(name)} { }

    ExprPtr instantiate_template_types(const TypeInfoPtr& template_type) const override;

protected:
    void init_impl(const vector<TypeInfoPtr>& types) override;
    TypeInfoPtr type_impl() const override;
    VarPtr evaluate_impl() const override;

private:
    const string& name() const { return token_.lexeme(); }

    VarPtr var_;
};

#endif //FENNEC_IDENTIFIER_H
