//
// Created by jaket on 26/10/2026.
//

#include "expressions/SwitchExpression.h"

#include "expressions/interface.h"
#include "runtime/ArgumentList.h"
#include "runtime/Type.h"
#include "runtime/utils/invoke.h"

namespace mxslc::expressions
{
    SwitchExpression::SwitchExpression(ExprPtr value_expr, vector<SwitchCase> cases, Token token)
        : Expression{std::move(token)}, value_expr_{std::move(value_expr)}, cases_{std::move(cases)}
    {

    }

    ExprPtr SwitchExpression::monomorphize(const TypePtr& template_type) const
    {
        ExprPtr value_expr = value_expr_->monomorphize(template_type);

        vector<SwitchCase> cases;
        for (const SwitchCase& case_ : cases_)
        {
            ExprPtr case_value_expr = case_.value_expr()->monomorphize(template_type);
            cases.emplace_back(case_.predicate(), std::move(case_value_expr));
        }

        return create_expression<SwitchExpression>(value_expr, cases, token_);
    }

    void SwitchExpression::init_subexpressions(const vector<TypePtr>& types)
    {
        value_expr_->init({Type::Int, Type::Float});

        for (const SwitchCase& case_ : cases_)
            case_.value_expr()->init(types);
    }

    void SwitchExpression::init_impl(const vector<TypePtr>& types)
    {

    }

    TypePtr SwitchExpression::type_impl() const
    {
        return cases_.front().value_expr()->type();
    }

    VarPtr SwitchExpression::evaluate_impl() const
    {
        ArgumentList args;

        size_t i = 0;
        for (const SwitchCase& case_ : cases_)
        {

            ++i;
        }

        args.add("which", value_expr_);

        return runtime_utils::invoke_function("switch", std::move(args));
    }

    string SwitchExpression::to_string() const
    {
        string result = "switch (" + value_expr_->to_string() + ")\n{";

        for (const SwitchCase& case_ : cases_)
            result += "\n    case " + case_.predicate() + ": " + case_.value_expr()->to_string();

        return result + "\n}";
    }
}
