//
// Created by jaket on 10/04/2026.
//

#ifndef MXSLC_BLOCKSTATEMENT_H
#define MXSLC_BLOCKSTATEMENT_H

#include "statements/Statement.h"

namespace mxslc::statements
{
    class BlockStatement final : public Statement
    {
    public:
        BlockStatement(vector<StmtPtr> body, Token token = {});

        StmtPtr monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;

    protected:
        void execute_impl() const override;

    private:
        vector<StmtPtr> body_;
    };
}

#endif //MXSLC_BLOCKSTATEMENT_H
