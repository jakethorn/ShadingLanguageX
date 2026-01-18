//
// Created by jaket on 11/01/2026.
//

#include "IndexingExpression.h"

#include "CompileError.h"
#include "values/BasicValue.h"
#include "runtime/Runtime.h"
#include "runtime/TypeInfo.h"

ExprPtr IndexingExpression::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    ExprPtr expr = expr_->instantiate_template_types(template_type);
    ExprPtr index = index_->instantiate_template_types(template_type);
    return std::make_unique<IndexingExpression>(runtime_, std::move(expr), std::move(index));
}

void IndexingExpression::init_subexpressions(const vector<TypeInfoPtr>&)
{
    expr_->init();
    index_->init(TypeInfo::Basic::Int);
}

TypeInfoPtr IndexingExpression::type_impl() const
{
    const ValuePtr index_val = index_->evaluate();
    if (const shared_ptr<BasicValue> basic_value = std::dynamic_pointer_cast<BasicValue>(index_val))
    {
        const int index = basic_value->get<int>();
        return expr_->type()->field_type(index);
    }

    throw CompileError{token_, "Cannot evaluate index at compile time"s};
}

ValuePtr IndexingExpression::evaluate_impl() const
{
    const ValuePtr expr_val = expr_->evaluate();
    const ValuePtr index_val = index_->evaluate();
    const shared_ptr<BasicValue> basic_value = std::dynamic_pointer_cast<BasicValue>(index_val);
    const int index = basic_value->get<int>();
    return expr_val->subvalue(index);
}
