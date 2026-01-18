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

    [[nodiscard]] ExprPtr instantiate_template_types(const TypeInfoPtr& template_type) const override;

protected:
    void init_subexpressions(const vector<TypeInfoPtr>& types) override;
    [[nodiscard]] TypeInfoPtr type_impl() const override;
    [[nodiscard]] ValuePtr evaluate_impl() const override;

private:
    void try_init_expressions(const vector<TypeInfoPtr>& types);
    [[nodiscard]] vector<TypeInfoPtr> index_types(const vector<TypeInfoPtr>& types, size_t index) const;

    vector<ExprPtr> exprs_;
    size_t initialised_expr_count_ = 0;
};

#endif //MXSLC_CONSTRUCTOR_H
