//
// Created by jaket on 27/03/2026.
//

#include "statements/VariableAssignment.h"

#include "expressions/Expression.h"
#include "runtime/variables/Variable.h"
#include "runtime/utils/monomorphize.h"
#include "statements/interface.h"

namespace mxslc::statements
{
    VariableAssignment::VariableAssignment(ExprPtr lhs_expr, ExprPtr rhs_expr, Token token)
        : Statement{std::move(token)}, lhs_exprs_{std::move(lhs_expr)}, rhs_expr_{std::move(rhs_expr)}
    {

    }

    VariableAssignment::VariableAssignment(vector<ExprPtr> lhs_exprs, ExprPtr rhs_expr, Token token)
        : Statement{std::move(token)}, lhs_exprs_{std::move(lhs_exprs)}, rhs_expr_{std::move(rhs_expr)}
    {

    }

    void VariableAssignment::set_attributes(AttributeList attrs)
    {
        rhs_expr_->set_attributes(std::move(attrs));
    }

    StmtPtr VariableAssignment::monomorphize(const TypePtr& template_type) const
    {
        auto&& [lhs, rhs] = runtime_utils::monomorphize_all(template_type, lhs_exprs_, rhs_expr_);
        return create_statement<VariableAssignment>(std::move(lhs), std::move(rhs), token_);
    }

    void VariableAssignment::execute_impl() const
    {
        // this covers both single and multi-variable assignment

        // initialise lhs
        for (const ExprPtr& lhs_expr : lhs_exprs_)
            lhs_expr->init();

        // get type of lhs
        TypePtr rhs_type;
        if (lhs_exprs_.size() == 1)
            rhs_type = lhs_exprs_[0]->type();
        else
            rhs_type = type_utils::type_of(lhs_exprs_);

        // initialize rhs using type of lhs
        rhs_expr_->init(rhs_type);

        // assign rhs to lhs
        if (lhs_exprs_.size() == 1)
        {
            lhs_exprs_[0]->assign(rhs_expr_->evaluate());
        }
        else
        {
            const VarPtr rhs_value = rhs_expr_->evaluate();
            for (size_t i = 0; i < lhs_exprs_.size(); ++i)
            {
                lhs_exprs_[i]->assign(rhs_value->child(i));
            }
        }
    }

    string VariableAssignment::to_string() const
    {
        return join(lhs_exprs_, ", ") + " = " + rhs_expr_->to_string() + ";";
    }
}
