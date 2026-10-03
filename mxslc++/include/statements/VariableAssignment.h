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
        VariableAssignment(ExprPtr lhs_expr, ExprPtr rhs_expr, Token token = {});
        VariableAssignment(vector<ExprPtr> lhs_exprs, ExprPtr rhs_expr, Token token = {});

        void set_attributes(AttributeList attrs) override;

        StmtPtr monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;

    protected:
        void execute_impl() const override;

    private:
        vector<ExprPtr> lhs_exprs_;
        ExprPtr rhs_expr_;
    };
}

#endif //MXSLC_VARIABLEASSIGNMENT_H
