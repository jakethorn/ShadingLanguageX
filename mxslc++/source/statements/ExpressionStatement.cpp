//
// Created by jaket on 08/04/2026.
//

//
// Created by jaket on 28/11/2025.
//

#include "statements/ExpressionStatement.h"

#include "expressions/Expression.h"
#include "expressions/IfExpression.h"
#include "expressions/interface.h"
#include "statements/interface.h"
#include "expressions/CompoundAssignment.h"
#include "expressions/IncrementOperator.h"
#include "runtime/variables/Variable.h"
#include "serialize/Serializer.h"
#include "serialize/decompile_hints.h"

namespace mxslc::statements
{
    ExpressionStatement::ExpressionStatement(ExprPtr expr)
        : Statement{expr->token()}, expr_{std::move(expr)}
    {

    }

    ExpressionStatement::~ExpressionStatement() = default;

    void ExpressionStatement::set_attributes(AttributeList attrs)
    {
        expr_->set_attributes(std::move(attrs));
    }

    StmtPtr ExpressionStatement::monomorphize(const TypePtr& template_type) const
    {
        ExprPtr expr = expr_->monomorphize(template_type);
        return create_statement<ExpressionStatement>(std::move(expr));
    }

    void ExpressionStatement::execute_impl() const
    {
        expr_->init();
        VarPtr _ = expr_->evaluate();

        if (const ExprPtr lvalue = assigned_expression())
        {
            if (const VarPtr var = serialize::HintRecorder::assigned_variable(lvalue))
                serializer().hints().bind_variable(var, var->name());
        }
    }

    ExprPtr ExpressionStatement::assigned_expression() const
    {
        if (const CompoundAssignmentPtr assignment = cast_expression<CompoundAssignment>(expr_))
            return assignment->lhs_expression();
        if (const IncrementOperatorPtr increment = cast_expression<IncrementOperator>(expr_))
            return increment->value_expression();
        return nullptr;
    }

    string ExpressionStatement::hint_skeleton() const
    {
        const ExprPtr lvalue = assigned_expression();
        if (lvalue == nullptr)
            return hints::PLACEHOLDER + ";";

        // compile-time values are not part of the graph, so their code is recorded, e.g., `x += 1.0;`
        const VarPtr var = serialize::HintRecorder::assigned_variable(lvalue);
        if ((var and var->is_compile_time()) or cast_expression<IncrementOperator>(expr_))
            return expr_->to_string() + ";";
        return lvalue->to_string() + " " + expr_->token().lexeme() + " " + hints::PLACEHOLDER + ";";
    }

    string ExpressionStatement::to_string() const
    {
        // statements beginning with `if` are if statements, so if expressions are wrapped in parentheses
        const bool is_if_expr = cast_expression<IfExpression>(expr_) != nullptr;
        const string expr_string = is_if_expr ? "(" + expr_->to_string() + ")" : expr_->to_string();
        return with_attributes(expr_->attributes(), expr_string + ";");
    }
}
