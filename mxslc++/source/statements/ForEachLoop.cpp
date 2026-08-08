//
// Created by jaket on 30/03/2026.
//

#include "statements/ForEachLoop.h"

#include "expressions/Expression.h"
#include "runtime/interface.h"
#include "runtime/Runtime.h"
#include "runtime/Type.h"
#include "runtime/Scope.h"
#include "runtime/variables/Variable.h"
#include "runtime/utils/monomorphize.h"
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
        auto&& [type, iter_expr, body] = runtime_utils::monomorphize_all(template_type, type_, iter_expr_, body_);
        return create_statement<ForEachLoop>(token_, mods_, std::move(type), name_, std::move(iter_expr), std::move(body));
    }

    void ForEachLoop::execute_impl() const
    {
        const TypePtr type = scope().resolve_type(type_);

        const TypePtr iter_type = Type::tuple(type);
        iter_expr_->init(iter_type);
        const VarPtr iter_value = iter_expr_->evaluate();
        if (iter_value->has_value())
            throw CompileError{"Expression is not iterable"};

        for (size_t i = 0; i < iter_value->child_count(); i++)
        {
            VarPtr next_value = iter_value->child(i);
            if (not next_value->type()->is_compatible_with(type))
                throw CompileError{"Field value does not match loop iterator type"};

            runtime().enter_scope();
            create_variable(mods_, type, next_value)->add_to_scope(name_);
            body_->execute();
            runtime().exit_scope();
        }
    }

    string ForEachLoop::to_string() const
    {
        string mods_string = mods_.to_string();
        if (not mods_string.empty())
            mods_string += " ";

        string result;
        result += mods_string;
        result += "for (";
        result += type_->to_string();
        result += " ";
        result += name_;
        result += " from ";
        result += iter_expr_->to_string();
        result += ")\n";
        result += body_->to_string();
        return result;
    }
}
