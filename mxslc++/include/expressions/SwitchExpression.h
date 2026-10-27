//
// Created by jaket on 26/10/2026.
//

#ifndef MXSLC_SWITCHEXPRESSION_H
#define MXSLC_SWITCHEXPRESSION_H

#include "expressions/Expression.h"

namespace mxslc::expressions
{
    template<typename T>
    using Indexed = unordered_map<size_t, T>;

    class SwitchCase final : public Monomorphizable<SwitchCase>, public Stringable
    {
    public:
        explicit SwitchCase(ExprPtr value_expr) : SwitchCase{{}, false, std::move(value_expr)} { }
        SwitchCase(const bool is_default, ExprPtr value_expr) : SwitchCase{{}, is_default, std::move(value_expr)} { }
        SwitchCase(vector<Token> predicate, ExprPtr value_expr) : SwitchCase{std::move(predicate), false, std::move(value_expr)} { }

        bool has_predicate() const { return not predicate_.empty(); }
        const vector<Token>& predicate() const { return predicate_; }
        bool is_default() const { return is_default_; }
        const ExprPtr& value_expr() const { return value_expr_; }

        SwitchCase monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;

    private:
        SwitchCase(vector<Token> predicate, const bool is_default, ExprPtr value_expr)
            : predicate_{std::move(predicate)}, is_default_{is_default}, value_expr_{std::move(value_expr)} { }

        vector<Token> predicate_;
        bool is_default_;
        ExprPtr value_expr_;
    };

    class SwitchExpression final : public Expression
    {
    public:
        SwitchExpression(ExprPtr which_expr, vector<SwitchCase> cases, Token token = {});

        ExprPtr monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;

    protected:
        void init_subexpressions(const vector<TypePtr>& types) override;
        void init_impl(const vector<TypePtr>& types) override;
        TypePtr type_impl() const override;
        VarPtr evaluate_impl() const override;

    private:
        void index_case_expressions();
        VarPtr runtime_evaluate(const VarPtr& which) const;

        ExprPtr which_expr_;
        vector<SwitchCase> cases_;

        Indexed<ExprPtr> indexed_case_exprs_;
    };
}

#endif //MXSLC_SWITCHEXPRESSION_H
