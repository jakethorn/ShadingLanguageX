//
// Created by jaket on 11/08/2026.
//

#include "expressions/SwitchExpression.h"

#include "expressions/interface.h"
#include "runtime/interface.h"
#include "runtime/Type.h"
#include "runtime/utils/invoke.h"
#include "runtime/variables/Variable.h"

namespace mxslc::expressions
{
    SwitchExpression::SwitchExpression(ExprPtr index_expr, vector<ExprPtr> case_exprs, ExprPtr default_expr, Token token)
        : Expression{std::move(token)}, index_expr_{std::move(index_expr)}, case_exprs_{std::move(case_exprs)}, default_expr_{std::move(default_expr)}
    {

    }

    ExprPtr SwitchExpression::monomorphize(const TypePtr& template_type) const
    {
        return create_expression<SwitchExpression>(
            runtime_utils::monomorphize(index_expr_, template_type),
            runtime_utils::monomorphize(case_exprs_, template_type),
            runtime_utils::monomorphize(default_expr_, template_type),
            token_
        );
    }

    void SwitchExpression::init_subexpressions(const vector<TypePtr>& types)
    {
        index_expr_->init({Type::Int, Type::Float});
        for (const ExprPtr& expr : case_exprs_)
            expr->init(types);
        if (default_expr_)
            default_expr_->init(types);

        for (const ExprPtr& expr : case_exprs_)
        {
            if (not expr->type()->equals(case_exprs_.front()->type(), /*field_names*/false))
                throw CompileError{"Switch cases do not evaluate to the same type"};
        }

        if (default_expr_)
        {
            if (not default_expr_->type()->equals(case_exprs_.front()->type(), /*field_names*/false))
                throw CompileError{"Switch default case do not evaluate to the same type"};
        }
    }

    void SwitchExpression::init_impl(const vector<TypePtr>& types)
    {
        if (case_exprs_.empty() and default_expr_ == nullptr)
            throw CompileError{"Switch must have at least one case"};
    }

    TypePtr SwitchExpression::type_impl() const
    {
        if (default_expr_)
            return default_expr_->type();
        return case_exprs_.front()->type();
    }

    namespace
    {
        VarPtr runtime_evaluate(const VarPtr& index, const vector<VarPtr>& cases)
        {
            if (cases.front()->has_value())
            {
                ArgumentList args{cases};
                args.add("which", index);
                return runtime_utils::invoke_function("switch", std::move(args));
            }
            else
            {
                vector<VarPtr> switch_children;
                switch_children.reserve(cases.front()->child_count());
                for (size_t i = 0; i < cases.front()->child_count(); ++i)
                {
                    vector<VarPtr> case_children;
                    case_children.reserve(cases.size());
                    for (const VarPtr& case_child : cases)
                        case_children.push_back(case_child->child(i));

                    switch_children.push_back(runtime_evaluate(index, case_children));
                }

                return create_variable(switch_children);
            }
        }
    }

    VarPtr SwitchExpression::evaluate_impl() const
    {
        const VarPtr index = index_expr_->evaluate();

        if (index->is_compile_time())
        {
            const int i = index->compile_time_value().cast<int>();

            if (i >= case_exprs_.size())
            {
                if (default_expr_)
                    return default_expr_->evaluate();
                else
                    throw CompileError{"Switch index (" + std::to_string(i) + ") is out of range of the number of cases (" + std::to_string(case_exprs_.size()) + ")"};
            }

            if (case_exprs_[i] == nullptr)
            {
                if (default_expr_)
                    return default_expr_->evaluate();
                else
                    throw CompileError{"Switch index (" + std::to_string(i) + ") does not match any of the cases"};
            }

            return case_exprs_[i]->evaluate();
        }

        vector<VarPtr> cases;
        cases.reserve(case_exprs_.size());
        for (const ExprPtr& case_expr : case_exprs_)
            cases.push_back(case_expr->evaluate());

        return runtime_evaluate(index, cases);
    }

    string SwitchExpression::to_string() const
    {
        return "";
    }
}
