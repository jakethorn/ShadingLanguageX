//
// Created by jaket on 11/01/2026.
//

#ifndef MXSLC_CONSTRUCTOR_H
#define MXSLC_CONSTRUCTOR_H

#include "Expression.h"

class UnnamedConstructor final : public Expression
{
public:
    UnnamedConstructor(const Runtime& runtime, vector<ExprPtr> exprs)
        : Expression{runtime, exprs.at(0)->token()}, exprs_{std::move(exprs)} { }

    [[nodiscard]] ExprPtr instantiate_template_types(const Type& template_type) const override;

protected:
    void init_subexpressions(const vector<Type>& types) override;
    void init_impl(const vector<Type>& types) override;
    [[nodiscard]] const Type& type_impl() const override;
    [[nodiscard]] ValuePtr evaluate_impl() const override;

private:
    void try_init_expressions(const vector<Type>& types);
    [[nodiscard]] vector<Type> index_type(const vector<Type>& types, size_t index) const;

    vector<ExprPtr> exprs_;
    size_t initialised_expr_count_ = 0;
    Type type_;
};

#endif //MXSLC_CONSTRUCTOR_H
