//
// Created by jaket on 15/06/2026.
//

#include "expressions/RangeExpression.h"

#include "expressions/interface.h"
#include "runtime/interface.h"
#include "runtime/Type.h"
#include "runtime/variables/Variable.h"
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

    namespace
    {
        void cast_to_target_type(const vector<TypePtr>& types, vector<Primitive>& result)
        {
            for (size_t i = 0; i < result.size(); ++i)
            {
                vector<TypePtr> subtypes;
                for (const TypePtr& type : types)
                {
                    if (type->field_count() == result.size())
                        subtypes.push_back(type->field_type(i));
                    if (type->is_tuple())
                        subtypes.push_back(type->template_type());
                }

                if (not Type::Int->is_in(subtypes) and Type::Float->is_in(subtypes))
                    result[i] = result[i].cast<float>();
            }
        }
    }

    void RangeExpression::init_subexpressions(const vector<TypePtr>& types)
    {
        const vector<TypePtr> subexpr_types{Type::Int, Type::Float};

        lower_expr_->init(subexpr_types);
        if (step_expr_)
            step_expr_->init(subexpr_types);
        upper_expr_->init(subexpr_types);
    }

    void RangeExpression::init_impl(const vector<TypePtr>& types)
    {
        Primitive lower = lower_expr_->evaluate()->compile_time_value();
        Primitive step = step_expr_ ? step_expr_->evaluate()->compile_time_value() : 1;
        Primitive upper = upper_expr_->evaluate()->compile_time_value();

        vector<Primitive> result;
        while (lower <= upper)
        {
            result.push_back(lower);
            lower += step;
        }

        cast_to_target_type(types, result);
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
