//
// Created by jaket on 10/04/2026.
//

#include "BlockStatement.h"

#include "utils/instantiate_template_types_utils.h"

BlockStatement::BlockStatement(const Runtime& runtime, Token token, vector<StmtPtr> body)
    : Statement{runtime, std::move(token)}, body_{std::move(body)} { }

StmtPtr BlockStatement::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    vector<StmtPtr> body = ::instantiate_template_types(body_, template_type);
    return std::make_unique<BlockStatement>(runtime_, token_, std::move(body));
}

void BlockStatement::execute_impl() const
{
    for (const StmtPtr& stmt : body_)
        stmt->execute();
}
