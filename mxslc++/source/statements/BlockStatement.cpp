//
// Created by jaket on 10/04/2026.
//

#include "statements/BlockStatement.h"

#include "runtime/utils/monomorphize.h"
#include "statements/interface.h"

namespace mxslc::statements
{
    BlockStatement::BlockStatement(Token token, vector<StmtPtr> body)
        : Statement{std::move(token)}, body_{std::move(body)} { }

    StmtPtr BlockStatement::monomorphize(const TypePtr& template_type) const
    {
        vector<StmtPtr> body = template_utils::monomorphize(body_, template_type);
        return create_statement<BlockStatement>(token_, std::move(body));
    }

    void BlockStatement::execute_impl() const
    {
        for (const StmtPtr& stmt : body_)
            stmt->execute();
    }
}
