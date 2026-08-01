//
// Created by jaket on 28/11/2025.
//

#ifndef FENNEC_IDENTIFIER_H
#define FENNEC_IDENTIFIER_H

#include "expressions/Expression.h"

namespace mxslc::expressions
{
    class Identifier final : public Expression
    {
    public:
        explicit Identifier(Token name) : Expression{std::move(name)}, name_{token_.lexeme()} { }

        const string& name() const { return name_; }

        ExprPtr monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;

    protected:
        void init_impl(const vector<TypePtr>& types) override;
        TypePtr type_impl() const override;
        VarPtr evaluate_impl() const override;

    private:
        string name_;
        VarPtr var_;
    };
}

#endif //FENNEC_IDENTIFIER_H
