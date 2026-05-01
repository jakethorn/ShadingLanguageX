//
// Created by jaket on 11/01/2026.
//

#include "IndexingExpression.h"

#include "values/BasicValue.h"
#include "runtime/Runtime.h"
#include "runtime/Type.h"
#include "runtime/Variable.h"

ExprPtr IndexingExpression::instantiate_template_types(const TypePtr& template_type) const
{
    ExprPtr expr = expr_->instantiate_template_types(template_type);
    ExprPtr index = index_expr_->instantiate_template_types(template_type);
    return std::make_unique<IndexingExpression>(std::move(expr), std::move(index));
}

void IndexingExpression::init_subexpressions(const vector<TypePtr>& types)
{
    expr_->init();
    index_expr_->init(Type::Int);
}

void IndexingExpression::init_impl(const vector<TypePtr>& types)
{
    const VarPtr index_val = index_expr_->evaluate();
    index_ = index_val->value_as<int>();
}

TypePtr IndexingExpression::type_impl() const
{
    return expr_->type()->field_type(index_);
}

VarPtr IndexingExpression::evaluate_impl() const
{
    return expr_->evaluate()->child(index_);
}
