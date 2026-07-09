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
        BlockStatement(Token token, vector<StmtPtr> body);

        StmtPtr monomorphize(const TypePtr& template_type) const override;

    protected:
        void execute_impl() const override;

    private:
        vector<StmtPtr> body_;
    };
}

#endif //MXSLC_BLOCKSTATEMENT_H
