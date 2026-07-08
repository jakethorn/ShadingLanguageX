//
// Created by jaket on 11/01/2026.
//

#ifndef MXSLC_INDEXINGOPERATOR_H
#define MXSLC_INDEXINGOPERATOR_H

#include "expressions/Expression.h"
#include "expressions/accessors/interface.h"
#include "expressions/accessors/Accessor.h"

namespace mxslc::expressions
{
    class IndexingOperator final : public Expression
    {
    public:
        IndexingOperator(ExprPtr value_expr, ExprPtr index_expr);

        ExprPtr instantiate_template_types(const TypePtr& template_type) const override;

    protected:
        void init_subexpressions(const vector<TypePtr>& types) override;
        void init_impl(const vector<TypePtr>& types) override;
        TypePtr type_impl() const override;
        VarPtr evaluate_impl() const override;

    private:
        ExprPtr value_expr_;
        ExprPtr index_expr_;

        AccessorPtr accessor_;
    };
}

#endif //MXSLC_INDEXINGOPERATOR_H
