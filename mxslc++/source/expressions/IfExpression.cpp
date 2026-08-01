//
// Created by jaket on 01/08/2026.
//

#include "expressions/IfExpression.h"

#include <cassert>

#include "expressions/interface.h"
#include "runtime/interface.h"
#include "runtime/Type.h"
#include "runtime/Variable.h"
#include "runtime/utils/invoke.h"

namespace mxslc::expressions
{
    IfExpression::IfExpression(ExprPtr cond_expr, ExprPtr then_expr, ExprPtr else_expr, Token token)
        : Expression{std::move(token)}, cond_expr_(std::move(cond_expr)), then_expr_(std::move(then_expr)), else_expr_(std::move(else_expr))
    {

    }

    ExprPtr IfExpression::monomorphize(const TypePtr& template_type) const
    {
        return create_expression<IfExpression>(
            runtime_utils::monomorphize(cond_expr_, template_type),
            runtime_utils::monomorphize(then_expr_, template_type),
            runtime_utils::monomorphize(else_expr_, template_type),
            token_
        );
    }

    void IfExpression::init_subexpressions(const vector<TypePtr>& types)
    {
        cond_expr_->init(Type::Bool);
        then_expr_->init(types);
        else_expr_->init(types);

        if (not then_expr_->type()->equals(else_expr_->type(), /*field_names*/false))
            throw CompileError{"Branch expressions do not evaluate to the same type"};
    }

    void IfExpression::init_impl(const vector<TypePtr>& types)
    {

    }

    TypePtr IfExpression::type_impl() const
    {
        return then_expr_->type();
    }

    namespace
    {
        VarPtr runtime_evaluate(const VarPtr& cond, const VarPtr& then, const VarPtr& else_)
        {
            if (then->type()->is_primitive())
            {
                return runtime_utils::invoke_function("ifequal", ArgumentList{cond, true, then, else_});
            }
            else
            {
                vector<VarPtr> children;
                children.reserve(then->child_count());
                for (size_t i = 0; i < then->child_count(); ++i)
                    children.push_back(runtime_evaluate(cond, then->child(i), else_->child(i)));

                return create_variable(children);
            }
        }
    }

    VarPtr IfExpression::evaluate_impl() const
    {
        const VarPtr cond = cond_expr_->evaluate();

        if (cond->is_basic<bool>())
            return cond->basic<bool>() ? then_expr_->evaluate() : else_expr_->evaluate();

        const VarPtr then = then_expr_->evaluate();
        const VarPtr else_ = else_expr_->evaluate();
        return runtime_evaluate(cond, then, else_);
    }

    string IfExpression::to_string() const
    {
        return "if (" + cond_expr_->to_string() + ") { " + then_expr_->to_string() + " }" + " else { " + else_expr_->to_string() + " }";
    }
}
