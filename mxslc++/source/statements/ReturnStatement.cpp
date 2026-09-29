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
#include "serialize/decompile_hints.h"
#include "serialize/HintRecorder.h"
#include "serialize/Serializer.h"

namespace mxslc::statements
{
    ReturnStatement::ReturnStatement(ExprPtr expr, Token token) : Statement{std::move(token)}, expr_{std::move(expr)}
    {

    }

    void ReturnStatement::set_attributes(AttributeList attrs)
    {
        if (expr_)
            expr_->set_attributes(std::move(attrs));
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
            VarPtr value = expr_->evaluate();
            if (serializer().hints().is_enabled())
                hint_value_ = serialize::HintRecorder::expression_skeleton(expr_, value);
            throw Branch{std::move(value)};
        }
        else
        {
            throw Branch{};
        }
    }

    string ReturnStatement::hint_skeleton() const
    {
        return expr_ ? "return " + hint_value_ + ";" : "return;";
    }

    string ReturnStatement::to_string() const
    {
        if (expr_)
            return with_attributes(expr_->attributes(), "return " + expr_->to_string() + ";");
        else
            return "return;";
    }
}
