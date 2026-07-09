//
// Created by jaket on 01/01/2026.
//

#ifndef FENNEC_PRINTSTATEMENT_H
#define FENNEC_PRINTSTATEMENT_H

#include "statements/Statement.h"

namespace mxslc::statements
{
    class PrintStatement final : public Statement
    {
    public:
        PrintStatement(Token token, vector<ExprPtr> exprs);

        StmtPtr monomorphize(const TypePtr& template_type) const override;

    protected:
        void execute_impl() const override;

    private:
        vector<ExprPtr> exprs_;
    };
}

#endif //FENNEC_PRINTSTATEMENT_H
