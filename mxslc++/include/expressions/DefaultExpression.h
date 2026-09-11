//
// Created by jaket on 11/09/2026.
//

#ifndef MXSLC_DEFAULTEXPRESSION_H
#define MXSLC_DEFAULTEXPRESSION_H

#include "expressions/Expression.h"

namespace mxslc::expressions
{
    class DefaultExpression final : public Expression
    {
    public:
        explicit DefaultExpression(TypePtr type, Token token = {});

        ExprPtr monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;

    protected:
        void init_impl(const vector<TypePtr>& types) override;
        TypePtr type_impl() const override;
        VarPtr evaluate_impl() const override;

    private:
        TypePtr type_;
    };
}

#endif //MXSLC_DEFAULTEXPRESSION_H
