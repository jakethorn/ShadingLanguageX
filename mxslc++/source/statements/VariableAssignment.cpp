//
// Created by jaket on 27/03/2026.
//

#include "statements/VariableAssignment.h"

#include "expressions/Expression.h"
#include "expressions/IfExpression.h"
#include "expressions/interface.h"
#include "runtime/variables/Variable.h"
#include "runtime/utils/monomorphize.h"
#include "expressions/Identifier.h"
#include "serialize/Serializer.h"
#include "serialize/decompile_hints.h"
#include "statements/interface.h"

namespace mxslc::statements
{
    VariableAssignment::VariableAssignment(Token token, ExprPtr lhs_expr, ExprPtr rhs_expr)
        : Statement{std::move(token)}, lhs_expr_{std::move(lhs_expr)}, rhs_expr_{std::move(rhs_expr)}
    {

    }

    VariableAssignment::~VariableAssignment() = default;

    void VariableAssignment::set_attributes(AttributeList attrs)
    {
        rhs_expr_->set_attributes(std::move(attrs));
    }

    StmtPtr VariableAssignment::monomorphize(const TypePtr& template_type) const
    {
        auto&& [lhs, rhs] = runtime_utils::monomorphize_all(template_type, lhs_expr_, rhs_expr_);
        return create_statement<VariableAssignment>(token_, std::move(lhs), std::move(rhs));
    }

    void VariableAssignment::execute_impl() const
    {
        lhs_expr_->init();
        rhs_expr_->init(lhs_expr_->type());
        lhs_expr_->assign(rhs_expr_->evaluate());

        if (const VarPtr var = serialize::HintRecorder::assigned_variable(lhs_expr_))
        {
            serializer().hints().bind_variable(var, var->name());

            // a copy of a variable, e.g., `uv = fragCoord;`
            const IdentifierPtr source = cast_expression<Identifier>(rhs_expr_);
            if (source and cast_expression<Identifier>(lhs_expr_))
                serializer().hints().copy_variable(var, source->variable());
        }
    }

    StmtPtr VariableAssignment::with_rhs(ExprPtr rhs_expr) const
    {
        return create_statement<VariableAssignment>(token_, lhs_expr_, std::move(rhs_expr));
    }

    string VariableAssignment::hint_skeleton() const
    {
        // compile-time values are not part of the graph, so their code is recorded, e.g., `x = foo();`, as is the code of
        // copies of variables, e.g., `uv = fragCoord;`
        const VarPtr var = serialize::HintRecorder::assigned_variable(lhs_expr_);
        const bool is_copy = cast_expression<Identifier>(lhs_expr_) and cast_expression<Identifier>(rhs_expr_);
        if ((var and var->is_compile_time()) or is_copy)
            return lhs_expr_->to_string() + " = " + rhs_expr_->to_string() + ";";

        string rhs = hints::PLACEHOLDER;
        if (const IdentifierPtr identifier = cast_expression<Identifier>(lhs_expr_))
            rhs = serialize::HintRecorder::expression_skeleton(rhs_expr_, identifier->variable());
        return lhs_expr_->to_string() + " = " + rhs + ";";
    }

    string VariableAssignment::to_string() const
    {
        // `x = if (cond) { y } else { x };` is printed as `x = if (cond) { y };`
        const IfExpressionPtr if_expr = cast_expression<IfExpression>(rhs_expr_);
        const string rhs_string = if_expr ? if_expr->to_string(lhs_expr_) : rhs_expr_->to_string();
        return with_attributes(rhs_expr_->attributes(), lhs_expr_->to_string() + " = " + rhs_string + ";");
    }
}
