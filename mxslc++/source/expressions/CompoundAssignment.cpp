//
// Created by jaket on 15/05/2026.
//

#include "expressions/CompoundAssignment.h"

#include "expressions/FunctionCall.h"
#include "expressions/interface.h"
#include "runtime/ArgumentList.h"
#include "runtime/Variable.h"

namespace mxslc::expressions
{
    CompoundAssignment::CompoundAssignment(ExprPtr lhs_expr, Token op, ExprPtr rhs_expr)
        : Expression{std::move(op)}, lhs_expr_{std::move(lhs_expr)}, rhs_expr_{std::move(rhs_expr)}
    {

    }

    ExprPtr CompoundAssignment::monomorphize(const TypePtr& template_type) const
    {
        ExprPtr lhs_expr = lhs_expr_->monomorphize(template_type);
        ExprPtr rhs_expr = rhs_expr_->monomorphize(template_type);
        return create_expression<CompoundAssignment>(std::move(lhs_expr), token_, std::move(rhs_expr));
    }

    void CompoundAssignment::init_impl(const vector<TypePtr>& types)
    {
        static const unordered_map<TokenType, string> OP_NAMES {
            {"+="s, "__add__"},
            {"-="s, "__sub__"},
            {"*="s, "__mul__"},
            {"/="s, "__div__"},
            {"%="s, "__mod__"},
            {"^="s, "__pow__"},
            {"&="s, "__and__"},
            {"|="s, "__or__"},
        };

        string dunder_name = OP_NAMES.at(token_.type());
        func_call_ = create_expression<FunctionCall>(std::move(dunder_name), ArgumentList{lhs_expr_, rhs_expr_});
        func_call_->init(types);
    }

    TypePtr CompoundAssignment::type_impl() const
    {
        return func_call_->type();
    }

    VarPtr CompoundAssignment::evaluate_impl() const
    {
        VarPtr value = func_call_->evaluate();
        lhs_expr_->evaluate()->copy(value);
        return value;
    }
}
