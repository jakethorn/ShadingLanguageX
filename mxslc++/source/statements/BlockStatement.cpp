//
// Created by jaket on 10/04/2026.
//

#include "statements/BlockStatement.h"

#include "../../include/runtime/utils/instantiate_template_types_utils.h"

namespace mxslc
{
BlockStatement::BlockStatement(Token token, vector<StmtPtr> body)
    : Statement{std::move(token)}, body_{std::move(body)} { }

StmtPtr BlockStatement::instantiate_template_types(const TypePtr& template_type) const
{
    vector<StmtPtr> body = mxslc::instantiate_template_types(body_, template_type);
    return std::make_unique<BlockStatement>(token_, std::move(body));
}

void BlockStatement::execute_impl() const
{
    for (const StmtPtr& stmt : body_)
        stmt->execute();
}
}

