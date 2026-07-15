//
// Created by jaket on 15/06/2026.
//

#include "expressions/RangeExpression.h"

#include "expressions/interface.h"
#include "runtime/interface.h"
#include "runtime/Type.h"
#include "runtime/Variable.h"
#include "runtime/utils/monomorphize.h"

namespace mxslc::expressions
{
    RangeExpression::RangeExpression(ExprPtr lower_expr, ExprPtr upper_expr)
        : RangeExpression{std::move(lower_expr), nullptr, std::move(upper_expr), Token{}}
    {

    }

    RangeExpression::RangeExpression(ExprPtr lower_expr, ExprPtr upper_expr, Token token)
        : RangeExpression{std::move(lower_expr), nullptr, std::move(upper_expr), std::move(token)}
    {

    }

    RangeExpression::RangeExpression(ExprPtr lower_expr, ExprPtr step_expr, ExprPtr upper_expr)
        : RangeExpression{std::move(lower_expr), std::move(step_expr), std::move(upper_expr), Token{}}
    {

    }

    RangeExpression::RangeExpression(ExprPtr lower_expr, ExprPtr step_expr, ExprPtr upper_expr, Token token)
        : Expression{std::move(token)}, lower_expr_{std::move(lower_expr)}, step_expr_{std::move(step_expr)}, upper_expr_{std::move(upper_expr)}
    {

    }

    ExprPtr RangeExpression::monomorphize(const TypePtr& template_type) const
    {
        return create_expression<RangeExpression>(
            runtime_utils::monomorphize(lower_expr_, template_type),
            runtime_utils::monomorphize(step_expr_, template_type),
            runtime_utils::monomorphize(upper_expr_, template_type),
            token_
        );
    }

    void RangeExpression::init_subexpressions(const vector<TypePtr>& types)
    {
        if (subexpr_type_ and not (subexpr_type_->is_auto() or subexpr_type_->is<float>() or subexpr_type_->is<int>()))
            throw CompileError{"Invalid range expression type: " + subexpr_type_->name()};

        const vector expr_types = subexpr_type_ ? vector{subexpr_type_} : vector{Type::Int, Type::Float};

        lower_expr_->init(expr_types);
        if (step_expr_)
            step_expr_->init(expr_types);
        upper_expr_->init(expr_types);
    }

    void RangeExpression::init_impl(const vector<TypePtr>& types)
    {
        Primitive lower = lower_expr_->evaluate()->basic();
        Primitive step = step_expr_ ? step_expr_->evaluate()->basic() : 1;
        Primitive upper = upper_expr_->evaluate()->basic();

        vector<Primitive> result;
        while (lower <= upper)
        {
            result.push_back(lower);
            lower += step;
        }

        range_ = create_variable(result);
    }

    TypePtr RangeExpression::type_impl() const
    {
        return range_->type();
    }

    VarPtr RangeExpression::evaluate_impl() const
    {
        return range_;
    }

    string RangeExpression::to_string() const
    {
        if (step_expr_)
            return lower_expr_->to_string() + ":" + step_expr_->to_string() + ":" + upper_expr_->to_string();
        else
            return lower_expr_->to_string() + ":" + upper_expr_->to_string();
    }
}
