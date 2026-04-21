//
// Created by jaket on 11/01/2026.
//

#include "IndexingExpression.h"

#include "CompileError.h"
#include "values/BasicValue.h"
#include "runtime/Runtime.h"
#include "runtime/TypeInfo.h"
#include "runtime/variables/ChildVariable.h"

ExprPtr IndexingExpression::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    ExprPtr expr = expr_->instantiate_template_types(template_type);
    ExprPtr index = index_expr_->instantiate_template_types(template_type);
    return std::make_unique<IndexingExpression>(runtime_, std::move(expr), std::move(index));
}

void IndexingExpression::init_subexpressions(const vector<TypeInfoPtr>& types)
{
    expr_->init();
    index_expr_->init(TypeInfo::Int);
}

void IndexingExpression::init_impl(const vector<TypeInfoPtr>& types)
{
    const VarPtr2 index_val = index_expr_->evaluate();
    index_ = index_val->value_as<int>();
}

TypeInfoPtr IndexingExpression::type_impl() const
{
    return expr_->type()->field_type(index_);
}

VarPtr2 IndexingExpression::evaluate_impl() const
{
    VarPtr2 var = expr_->evaluate();
    return var->child(index_);
}
