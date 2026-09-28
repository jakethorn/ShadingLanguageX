//
// Created by jaket on 10/04/2026.
//

#include "statements/BlockStatement.h"

#include "runtime/utils/monomorphize.h"
#include "statements/interface.h"
#include "utils/string_utils.h"

namespace mxslc::statements
{
    BlockStatement::BlockStatement(vector<StmtPtr> body, Token token)
        : Statement{std::move(token)}, body_{std::move(body)} { }

    StmtPtr BlockStatement::monomorphize(const TypePtr& template_type) const
    {
        vector<StmtPtr> body = runtime_utils::monomorphize(body_, template_type);
        return create_statement<BlockStatement>(std::move(body), token_);
    }

    void BlockStatement::execute_impl() const
    {
        for (const StmtPtr& stmt : body_)
            stmt->execute();
    }

    string BlockStatement::to_string() const
    {
        if (body_.empty())
            return "{\n}";
        return "{\n" + string_utils::indent(join_statements(body_)) + "\n}";
    }
}
