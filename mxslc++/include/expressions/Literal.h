//
// Created by jaket on 27/11/2025.
//

#ifndef FENNEC_LITERAL_H
#define FENNEC_LITERAL_H

#include "expressions/Expression.h"
#include "Primitive.h"

namespace mxslc::expressions
{
    class Literal final : public Expression
    {
    public:
        explicit Literal(Token token) : Expression{std::move(token)}, value_{token_.literal()} { }
        explicit Literal(Primitive value) : value_{std::move(value)} { }

        const Primitive& value() const { return value_; }

        ExprPtr monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;
        Precedence precedence() const override;

    protected:
        void init_impl(const vector<TypePtr>& types) override;
        TypePtr type_impl() const override;
        VarPtr evaluate_impl() const override;

    private:
        Primitive value_;
        ValuePtr default_value_;
    };
}

#endif //FENNEC_LITERAL_H
