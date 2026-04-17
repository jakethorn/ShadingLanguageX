//
// Created by jaket on 30/03/2026.
////
// Created by jaket on 30/03/2026.
//

#include "ForEachLoop.h"

#include "expressions/Expression.h"
#include "CompileError.h"
#include "runtime/Runtime.h"
#include "runtime/TypeInfo.h"
#include "../runtime/variables/Variable.h"
#include "runtime/Scope.h"
#include "runtime/Variable2.h"
#include "utils/instantiate_template_types_utils.h"
#include "values/Value.h"

ForEachLoop::ForEachLoop(Token token, ModifierList mods, TypeInfoPtr type, string name, ExprPtr iter_expr, StmtPtr body)
    : Statement{std::move(token)},
    mods_{std::move(mods)},
    type_{std::move(type)},
    name_{std::move(name)},
    iter_expr_{std::move(iter_expr)},
    body_{std::move(body)}
{

}

StmtPtr ForEachLoop::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    TypeInfoPtr type = type_->instantiate_template_types(template_type);
    ExprPtr iter_expr = ::instantiate_template_types(iter_expr_, template_type);
    StmtPtr body = body_->instantiate_template_types(template_type);
    return std::make_unique<ForEachLoop>(token_, mods_, std::move(type), name_, std::move(iter_expr), std::move(body));
}

void ForEachLoop::execute_impl() const
{
    const TypeInfoPtr type = runtime().scope().resolve_type(type_);

    iter_expr_->init();
    if (not iter_expr_->type()->has_fields())
        throw CompileError{"Value not iterable"s};
    const VarPtr2 iter_value = iter_expr_->evaluate();

    for (size_t i = 0; i < iter_value->child_count(); i++)
    {
        VarPtr2 next_value = iter_value->child(i);
        if (not next_value->type()->is_compatible(type))
            throw CompileError{"Field value does not match loop iterator type"s};

        runtime().enter_inline_scope();
        VarPtr2 var = std::make_shared<Variable2>(mods_, type, name_, next_value);
        runtime().scope().add_variable(std::move(var));
        body_->execute();
        runtime().exit_scope();
    }
}
