//
// Created by jaket on 25/03/2026.
//

#include "DotExpression.h"

#include "CompileError.h"
#include "values/BasicValue.h"
#include "runtime/Runtime.h"
#include "runtime/TypeInfo.h"

ExprPtr DotExpression::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    ExprPtr expr = expr_->instantiate_template_types(template_type);
    return std::make_unique<DotExpression>(runtime_, std::move(expr), token_);
}

void DotExpression::init_subexpressions(const vector<TypeInfoPtr>&)
{
    expr_->init();
}

TypeInfoPtr DotExpression::type_impl() const
{
    return expr_->type()->field_type(token_);
}

ValuePtr DotExpression::evaluate_impl() const
{
    const ValuePtr expr_val = expr_->evaluate();
    return expr_val->subvalue(token_);
}
