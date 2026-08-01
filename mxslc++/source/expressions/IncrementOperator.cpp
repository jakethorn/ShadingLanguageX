//
// Created by jaket on 05/05/2026.
//

#include "expressions/IncrementOperator.h"

#include "expressions/FunctionCall.h"
#include "expressions/interface.h"
#include "runtime/Variable.h"

namespace mxslc::expressions
{
    IncrementOperator::IncrementOperator(ExprPtr value_expr, Token op, const bool prefix)
        : Expression{std::move(op)}, value_expr_{std::move(value_expr)}, prefix_{prefix}, increment_{token_ == TokenType::Increment}
    {

    }

    ExprPtr IncrementOperator::monomorphize(const TypePtr& template_type) const
    {
        ExprPtr value_expr = value_expr_->monomorphize(template_type);
        return create_expression<IncrementOperator>(std::move(value_expr), token_, prefix_);
    }

    void IncrementOperator::init_subexpressions(const vector<TypePtr>& types)
    {
        value_expr_->init(types);
    }

    TypePtr IncrementOperator::type_impl() const
    {
        return value_expr_->type();
    }

    VarPtr IncrementOperator::evaluate_impl() const
    {
        const VarPtr value = value_expr_->evaluate();
        VarPtr original_value = value->copy();

        string dunder_name = increment_ ? "__inc__" : "__dec__";
        const ExprPtr func_call = create_expression<FunctionCall>(std::move(dunder_name), ArgumentList{value_expr_});

        func_call->init(value->type());
        VarPtr incremented_value = func_call->evaluate();
        value->copy(incremented_value);

        if (prefix_)
            return incremented_value;
        else
            return original_value;
    }

    string IncrementOperator::to_string() const
    {
        const string value = value_expr_->to_string();
        const string op = token_.lexeme();
        return prefix_ ? op + value : value + op;
    }
}
