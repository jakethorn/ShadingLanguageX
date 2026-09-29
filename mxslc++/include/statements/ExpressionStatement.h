//
// Created by jaket on 08/04/2026.
//

#ifndef MXSLC_EXPRESSIONSTATEMENT_H
#define MXSLC_EXPRESSIONSTATEMENT_H

#include "statements/Statement.h"

namespace mxslc::statements
{
    class ExpressionStatement final : public Statement
    {
    public:
        explicit ExpressionStatement(ExprPtr expr);
        ~ExpressionStatement() override;

        void set_attributes(AttributeList attrs) override;

        StmtPtr monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;

    protected:
        void execute_impl() const override;

    private:
        ExprPtr expr_;
    };
}

#endif //MXSLC_EXPRESSIONSTATEMENT_H
