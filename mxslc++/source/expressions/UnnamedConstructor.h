//
// Created by jaket on 11/01/2026.
//

#ifndef MXSLC_CONSTRUCTOR_H
#define MXSLC_CONSTRUCTOR_H

#include "Expression.h"

class UnnamedConstructor final : public Expression
{
public:
    UnnamedConstructor(Token token, vector<ExprPtr> exprs)
        : Expression{std::move(token)}, exprs_{std::move(exprs)} { }

    ExprPtr instantiate_template_types(const TypeInfoPtr& template_type) const override;

protected:
    void init_subexpressions(const vector<TypeInfoPtr>& types) override;
    TypeInfoPtr type_impl() const override;
    VarPtr2 evaluate_impl() const override;

private:
    bool expressions_are_initialized();
    void try_init_expressions(const vector<TypeInfoPtr>& types);
    vector<TypeInfoPtr> index_types(const vector<TypeInfoPtr>& types, size_t index) const;

    vector<ExprPtr> exprs_;
    size_t initialized_expr_count_ = 0;
};

#endif //MXSLC_CONSTRUCTOR_H
