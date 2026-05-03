//
// Created by jaket on 30/03/2026.
//

#include "ForRangeLoop.h"

#include "CompileError.h"
#include "expressions/Expression.h"
#include "runtime/Runtime.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"
#include "runtime/Variable.h"
#include "utils/instantiate_template_types_utils.h"
#include "values/Value.h"

ForRangeLoop::ForRangeLoop(Token token, ModifierList mods, TypePtr type, string name, ExprPtr lower_expr, ExprPtr step_expr, ExprPtr upper_expr, StmtPtr body)
    : Statement{std::move(token)},
    mods_{std::move(mods)},
    type_{std::move(type)},
    name_{std::move(name)},
    lower_expr_{std::move(lower_expr)},
    step_expr_{std::move(step_expr)},
    upper_expr_{std::move(upper_expr)},
    body_{std::move(body)}
{

}

StmtPtr ForRangeLoop::instantiate_template_types(const TypePtr& template_type) const
{
    TypePtr type = type_->instantiate_template_types(template_type);
    ExprPtr lower_expr = ::instantiate_template_types(lower_expr_, template_type);
    ExprPtr step_expr = ::instantiate_template_types(step_expr_, template_type);
    ExprPtr upper_expr = ::instantiate_template_types(upper_expr_, template_type);
    StmtPtr body = body_->instantiate_template_types(template_type);
    return std::make_unique<ForRangeLoop>(token_, mods_, std::move(type), name_, std::move(lower_expr), std::move(step_expr), std::move(upper_expr), std::move(body));
}

void ForRangeLoop::execute_impl() const
{
    const TypePtr type = scope().resolve_type(type_);
    if (type->is<int>())
        execute_with<int>();
    else if (type->is<float>())
        execute_with<float>();
    else
        throw CompileError{"Unsupported loop iterator type"s};
}
