//
// Created by jaket on 11/01/2026.
//

#include "IndexingOperator.h"

#include "accessors/ComponentAccessor.h"
#include "accessors/FieldAccessor.h"
#include "runtime/Runtime.h"
#include "runtime/Type.h"
#include "runtime/Variable.h"

ExprPtr IndexingOperator::instantiate_template_types(const TypePtr& template_type) const
{
    ExprPtr expr = value_expr_->instantiate_template_types(template_type);
    ExprPtr index = index_expr_->instantiate_template_types(template_type);
    return std::make_unique<IndexingOperator>(std::move(expr), std::move(index));
}

void IndexingOperator::init_subexpressions(const vector<TypePtr>& types)
{
    value_expr_->init();
    index_expr_->init(Type::Int);
}

void IndexingOperator::init_impl(const vector<TypePtr>& types)
{
    if (value_expr_->type()->is_primitive())
    {
        accessor_ = std::make_unique<ComponentAccessor>(value_expr_, index_expr_);
    }
    else
    {
        VarPtr var = value_expr_->evaluate();
        const VarPtr index_val = index_expr_->evaluate();
        const int index = index_val->value_as<int>();
        accessor_ = std::make_unique<FieldAccessor>(std::move(var), index);
    }
}

TypePtr IndexingOperator::type_impl() const
{
    return accessor_->type();
}

VarPtr IndexingOperator::evaluate_impl() const
{
    return accessor_->evaluate();
}
