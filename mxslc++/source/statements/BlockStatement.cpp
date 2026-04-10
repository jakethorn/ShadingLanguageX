//
// Created by jaket on 10/04/2026.
//

#include "BlockStatement.h"

#include "expressions/Expression.h"
#include "runtime/Runtime.h"
#include "utils/instantiate_template_types_utils.h"
#include "values/Value.h"

BlockStatement::BlockStatement(const Runtime& runtime, vector<StmtPtr> body)
    : Statement{runtime}, body_{std::move(body)} { }

StmtPtr BlockStatement::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    vector<StmtPtr> body = ::instantiate_template_types(body_, template_type);
    return std::make_unique<BlockStatement>(runtime_, std::move(body));
}

void BlockStatement::execute() const
{
    for (const StmtPtr& stmt : body_)
        stmt->execute();
}
