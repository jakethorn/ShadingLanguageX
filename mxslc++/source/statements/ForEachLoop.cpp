//
// Created by jaket on 30/03/2026.
//

#include "statements/ForEachLoop.h"

#include "expressions/Expression.h"
#include "runtime/Runtime.h"
#include "runtime/Type.h"
#include "runtime/Scope.h"
#include "runtime/Variable.h"
#include "runtime/utils/monomorphize.h"
#include "serialize/values/Value.h"
#include "errors/CompileError.h"
#include "statements/interface.h"

namespace mxslc::statements
{
    ForEachLoop::ForEachLoop(Token token, ModifierList mods, TypePtr type, string name, ExprPtr iter_expr, StmtPtr body)
        : Statement{std::move(token)},
        mods_{std::move(mods)},
        type_{std::move(type)},
        name_{std::move(name)},
        iter_expr_{std::move(iter_expr)},
        body_{std::move(body)}
    {

    }

    StmtPtr ForEachLoop::monomorphize(const TypePtr& template_type) const
    {
        auto&& [type, iter_expr, body] = template_utils::monomorphize_all(template_type, type_, iter_expr_, body_);
        return create_statement<ForEachLoop>(token_, mods_, std::move(type), name_, std::move(iter_expr), std::move(body));
    }

    void ForEachLoop::execute_impl() const
    {
        const TypePtr type = scope().resolve_type(type_);

        iter_expr_->set_subexpression_type(type);
        iter_expr_->init();
        if (not iter_expr_->type()->has_fields())
            throw CompileError{"Value not iterable"};
        const VarPtr iter_value = iter_expr_->evaluate();

        for (size_t i = 0; i < iter_value->child_count(); i++)
        {
            VarPtr next_value = iter_value->child(i);
            if (not next_value->type()->is_compatible(type))
                throw CompileError{"Field value does not match loop iterator type"};

            runtime().enter_scope();
            Variable::create(mods_, type, next_value)->add_to_scope(name_);
            body_->execute();
            runtime().exit_scope();
        }
    }
}
