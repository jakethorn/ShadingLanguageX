//
// Created by jaket on 26/10/2026.
//

#ifndef MXSLC_SWITCHEXPRESSION_H
#define MXSLC_SWITCHEXPRESSION_H

#include "expressions/Expression.h"

namespace mxslc::expressions
{
    class SwitchCase : Stringable
    {
    public:
        explicit SwitchCase(ExprPtr value_expr) : SwitchCase{"", std::move(value_expr)} { }
        SwitchCase(string predicate, ExprPtr value_expr) : predicate_{std::move(predicate)}, value_expr_{std::move(value_expr)} { }

        bool has_predicate() const { return not predicate_.empty(); }
        const string& predicate() const { return predicate_; }
        const ExprPtr& value_expr() const { return value_expr_; }

        string to_string() const override
        {
            if (has_predicate())
                return "case " + predicate_ + ": " + value_expr_->to_string();
            else
                return value_expr_->to_string();
        }

    private:
        string predicate_;
        ExprPtr value_expr_;
    };

    class SwitchExpression final : public Expression
    {
    public:
        explicit SwitchExpression(ExprPtr value_expr, vector<SwitchCase> cases, Token token = {});

        ExprPtr monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;

    protected:
        void init_subexpressions(const vector<TypePtr>& types) override;
        void init_impl(const vector<TypePtr>& types) override;
        TypePtr type_impl() const override;
        VarPtr evaluate_impl() const override;

    private:
        ExprPtr value_expr_;
        vector<SwitchCase> cases_;
    };
}

#endif //MXSLC_SWITCHEXPRESSION_H
