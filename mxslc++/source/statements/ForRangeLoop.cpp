//
// Created by jaket on 30/03/2026.
//

#include "ForRangeLoop.h"

#include "CompileError.h"
#include "expressions/Expression.h"
#include "runtime/Runtime.h"
#include "runtime/TypeInfo.h"
#include "runtime/Variable.h"
#include "values/BasicValue.h"
#include "values/Value.h"

ForRangeLoop::ForRangeLoop(const Runtime& runtime, Token token, TypeInfoPtr type, string name, ExprPtr lower_expr, ExprPtr step_expr, ExprPtr upper_expr, StmtPtr body)
    : Statement{runtime, std::move(token)},
    type_{std::move(type)},
    name_{std::move(name)},
    lower_expr_{std::move(lower_expr)},
    step_expr_{std::move(step_expr)},
    upper_expr_{std::move(upper_expr)},
    body_{std::move(body)}
{

}

StmtPtr ForRangeLoop::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    TypeInfoPtr type = type_->instantiate_template_types(template_type);
    ExprPtr lower_expr = ::instantiate_template_types(lower_expr_, template_type);
    ExprPtr step_expr = ::instantiate_template_types(step_expr_, template_type);
    ExprPtr upper_expr = ::instantiate_template_types(upper_expr_, template_type);
    StmtPtr body = body_->instantiate_template_types(template_type);
    return std::make_unique<ForRangeLoop>(runtime_, token_, std::move(type), name_, std::move(lower_expr), std::move(step_expr), std::move(upper_expr), std::move(body));
}

void ForRangeLoop::execute_impl() const
{
    const TypeInfoPtr type = runtime_.scope().resolve_type(type_);

    lower_expr_->init(type);
    if (step_expr_)
        step_expr_->init(type);
    upper_expr_->init(type);

    const ValuePtr lower_val = lower_expr_->evaluate();
    const ValuePtr step_val = step_expr_ ? step_expr_->evaluate() : nullptr;
    const ValuePtr upper_val = upper_expr_->evaluate();

    if (type == TypeInfo::Int)
    {
        int lower = lower_val->as_int();
        const int step = step_val ? step_val->as_int() : 1;
        const int upper = upper_val->as_int();

        while (lower <= upper)
        {
            runtime_.enter_inline_scope();
            VarPtr var = std::make_shared<Variable>(ModifierList{}, name_, std::make_shared<BasicValue>(lower));
            runtime_.scope().add_variable(std::move(var));
            body_->execute();
            runtime_.exit_scope();
            lower += step;
        }
    }
    else if (type == TypeInfo::Float)
    {
        float lower = lower_val->as_float();
        const float step = step_val ? step_val->as_float() : 1;
        const float upper = upper_val->as_float();

        while (lower <= upper)
        {
            runtime_.enter_inline_scope();
            VarPtr var = std::make_shared<Variable>(ModifierList{}, name_, std::make_shared<BasicValue>(lower));
            runtime_.scope().add_variable(std::move(var));
            body_->execute();
            runtime_.exit_scope();
            lower += step;
        }
    }
    else
    {
        throw CompileError{"Unsupported loop iterator type"s};
    }
}
