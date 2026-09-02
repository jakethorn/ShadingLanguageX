//
// Created by jaket on 01/09/2026.
//

#include "statements/ReturnStatement.h"

#include "expressions/Expression.h"
#include "runtime/Function.h"
#include "runtime/Runtime.h"
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
        // unwind stack
        while (scope().function() == nullptr)
        {
            if (scope().parent() == nullptr)
                throw CompileError{"Stack unwind error. There is likely a return statement in an invalid location."};
            runtime().exit_scope();
        }

        const FuncPtr func = scope().function();

        if (func->is_void() and expr_)
            throw CompileError{"Cannot return a value from void function '" + func->name() + "'"};

        if (not func->is_void() and not expr_)
            throw CompileError{"Must return a value from non-void function '" + func->name() + "'"};

        if (expr_)
        {
            expr_->init(func->return_type());
            throw Branch{expr_->evaluate()};
        }
        else
        {
            throw Branch{};
        }
    }

    string ReturnStatement::to_string() const
    {
        if (expr_)
            return "return " + expr_->to_string() + ";";
        else
            return "return;";
    }
}
