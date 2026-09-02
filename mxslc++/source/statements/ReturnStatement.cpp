//
// Created by jaket on 01/09/2026.
//

#include "statements/ReturnStatement.h"

#include "expressions/Expression.h"
#include "runtime/Function.h"
#include "runtime/Scope.h"
#include "runtime/variables/Variable.h"
#include "runtime/utils/monomorphize.h"
#include "statements/interface.h"
#include "serialize/values/Value.h"

namespace mxslc::statements
{
    ReturnStatement::ReturnStatement(ExprPtr expr, Token token) : Statement{std::move(token)}, expr_{std::move(expr)}
    {

    }

    StmtPtr ReturnStatement::monomorphize(const TypePtr& template_type) const
    {
        ExprPtr expr = runtime_utils::monomorphize(expr_, template_type);
        return create_statement<ReturnStatement>(std::move(expr), token_);
    }

    void ReturnStatement::execute_impl() const
    {
        const FuncPtr func = scope().function();

        if (func->is_void())
            throw CompileError{"Cannot return from void function '" + func->name() + "'"};

        expr_->init(func->return_type());
        throw ReturnClause{expr_->evaluate()};
    }

    string ReturnStatement::to_string() const
    {
        return "return " + expr_->to_string() + ";";
    }
}
