//
// Created by jaket on 25/03/2026.
//

#include "DotExpression.h"

#include "Identifier.h"
#include "runtime/Runtime.h"
#include "runtime/Type.h"
#include "runtime/Variable.h"
#include "values/Value.h"

ExprPtr DotExpression::instantiate_template_types(const TypePtr& template_type) const
{
    ExprPtr expr = expr_->instantiate_template_types(template_type);
    return std::make_unique<DotExpression>(std::move(expr), token_);
}

void DotExpression::init_subexpressions(const vector<TypePtr>& types)
{
    expr_->init();
}

TypePtr DotExpression::type_impl() const
{
    return expr_->type()->field_type(property());
}

VarPtr DotExpression::evaluate_impl() const
{
    return expr_->evaluate()->child(property());
}
