//
// Created by jaket on 30/03/2026.
////
// Created by jaket on 30/03/2026.
//

#include "ForEachLoop.h"
#include "expressions/Expression.h"
#include "runtime/Runtime.h"
#include "runtime/TypeInfo.h"
#include "runtime/Variable.h"
#include "values/BasicValue.h"
#include "values/Value.h"

ForEachLoop::ForEachLoop(const Runtime& runtime, TypeInfoPtr type, Token name, ExprPtr iter_expr, vector<StmtPtr> body)
    : Statement{runtime},
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
    vector<StmtPtr> body = ::instantiate_template_types(body_, template_type);
    return std::make_unique<ForEachLoop>(runtime_, std::move(type), name_, std::move(iter_expr), std::move(body));
}

void ForEachLoop::execute() const
{
    const TypeInfoPtr type = runtime_.scope().resolve_type(type_);

    iter_expr_->init();

    if (not iter_expr_->type()->has_fields())
    {
        throw CompileError{name_, "Value not iterable"s};
    }

    const ValuePtr iter_val = iter_expr_->evaluate();
    for (size_t i = 0; i < iter_val->subvalue_count(); i++)
    {
        runtime_.enter_scope();

        ValuePtr subvalue = iter_val->subvalue(i);
        if (not subvalue->type()->is_equal(type))
            throw CompileError{name_, "Field value does not match loop iterator type"s};

        VarPtr var = std::make_shared<Variable>(ModifierList{}, name_, subvalue);
        runtime_.scope().add_variable(std::move(var));

        for (const StmtPtr& stmt : body_)
            stmt->execute();

        runtime_.exit_scope();
    }
}
