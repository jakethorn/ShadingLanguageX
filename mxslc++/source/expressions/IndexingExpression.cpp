//
// Created by jaket on 11/01/2026.
//

#include "IndexingExpression.h"

#include "CompileError.h"
#include "utils/str_utils.h"
#include "values/BasicValue.h"

ExprPtr IndexingExpression::instantiate_template_types(const Type& template_type) const
{
    ExprPtr expr = expr_->instantiate_template_types(template_type);
    ExprPtr index = index_->instantiate_template_types(template_type);
    return std::make_unique<IndexingExpression>(runtime_, std::move(expr), std::move(index));
}

void IndexingExpression::init_subexpressions(const vector<Type>&)
{
    expr_->init();
    index_->init(Type::Int);
}

void IndexingExpression::init_impl(const vector<Type>&)
{
    const ValuePtr expr_val = expr_->evaluate();
    const ValuePtr index_val = index_->evaluate();
    if (const shared_ptr<BasicValue> basic_value = std::dynamic_pointer_cast<BasicValue>(index_val))
    {
        const int index = basic_value->get<int>();
        const size_t count = expr_val->subvalue_count();
        if (index < count)
        {
            const ValuePtr subval = expr_val->subvalue(index);
            type_ = subval->type();
        }
        else
        {
            throw CompileError{token_, "Index out of bounds (subvalue_count: " + str(count) + ", index: " + str(index) + ")"};
        }
    }
    else
    {
        throw CompileError{token_, "Cannot evaluate index at compile time"};
    }
}

const Type& IndexingExpression::type_impl() const
{
    return type_;
}

ValuePtr IndexingExpression::evaluate_impl() const
{
    const ValuePtr expr_val = expr_->evaluate();
    const ValuePtr index_val = index_->evaluate();
    const shared_ptr<BasicValue> basic_value = std::dynamic_pointer_cast<BasicValue>(index_val);
    const int index = basic_value->get<int>();
    return expr_val->subvalue(index);
}
