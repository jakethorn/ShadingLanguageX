//
// Created by jaket on 14/06/2026.
//

#ifndef MXSLC_TYPEOFOPERATOR_H
#define MXSLC_TYPEOFOPERATOR_H

#include "Expression.h"

class TypeOfOperator final : public Expression
{
public:
    explicit TypeOfOperator(ExprPtr expr);
    TypeOfOperator(ExprPtr expr, Token token);

    ExprPtr instantiate_template_types(const TypePtr& template_type) const override;

protected:
    void init_subexpressions(const vector<TypePtr>& types) override;
    void init_impl(const vector<TypePtr>& types) override;
    TypePtr type_impl() const override;
    VarPtr evaluate_impl() const override;

private:
    ExprPtr expr_;
    TypePtr type_;
};

#endif //MXSLC_TYPEOFOPERATOR_H
