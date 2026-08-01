//
// Created by jaket on 14/06/2026.
//

#ifndef MXSLC_TYPEOFOPERATOR_H
#define MXSLC_TYPEOFOPERATOR_H

#include "expressions/Expression.h"

namespace mxslc::expressions
{
    class TypeOfOperator final : public Expression
    {
    public:
        explicit TypeOfOperator(ExprPtr expr);
        TypeOfOperator(ExprPtr expr, Token token);
        TypeOfOperator(ExprPtr expr, TypePtr template_type, Token token);

        ExprPtr monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;

    protected:
        void init_subexpressions(const vector<TypePtr>& types) override;
        void init_impl(const vector<TypePtr>& types) override;
        TypePtr type_impl() const override;
        VarPtr evaluate_impl() const override;

    private:
        ExprPtr expr_;
        TypePtr expr_type_;
        TypePtr template_type_;
        TypePtr op_type_;
    };
}

#endif //MXSLC_TYPEOFOPERATOR_H
