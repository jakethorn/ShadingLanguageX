//
// Created by jaket on 27/03/2026.
//

#include "statements/VariableAssignment.h"

#include "expressions/Expression.h"
#include "runtime/Variable.h"
#include "runtime/utils/monomorphize.h"
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
        const VarPtr lhs = lhs_expr_->evaluate();
        const VarPtr rhs = rhs_expr_->evaluate();
        lhs->copy(rhs);
    }
}
