//
// Created by jaket on 27/03/2026.
//

#ifndef MXSLC_VARIABLEASSIGNMENT_H
#define MXSLC_VARIABLEASSIGNMENT_H

#include "statements/Statement.h"

namespace mxslc::statements
{
    class VariableAssignment final : public Statement
    {
    public:
        VariableAssignment(Token token, ExprPtr lhs_expr, ExprPtr rhs_expr);
        ~VariableAssignment() override;

        const ExprPtr& lhs_expression() const { return lhs_expr_; }
        const ExprPtr& rhs_expression() const { return rhs_expr_; }
        StmtPtr with_rhs(ExprPtr rhs_expr) const;

        void set_attributes(AttributeList attrs) override;

        StmtPtr monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;

        bool is_hinted() const override { return true; }
        string hint_skeleton() const override;

    protected:
        void execute_impl() const override;

    private:
        ExprPtr lhs_expr_;
        ExprPtr rhs_expr_;
    };
}

#endif //MXSLC_VARIABLEASSIGNMENT_H
