//
// Created by jaket on 11/01/2026.
//

#include "IndexingExpression.h"

#include "CompileError.h"
#include "values/BasicValue.h"
#include "runtime/Runtime.h"
#include "runtime/TypeInfo.h"
#include "runtime/Variable.h"

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
    const ValuePtr index_val = index_expr_->evaluate();
    if (const shared_ptr<BasicValue> basic_value = std::dynamic_pointer_cast<BasicValue>(index_val))
        index_ = basic_value->get<int>();
    else
        throw CompileError{token_, "Indexing expression could not be evaluated at compile time"s};
}

TypeInfoPtr IndexingExpression::type_impl() const
{
    return expr_->type()->field_type(index_);
}

VarPtr IndexingExpression::variable() const
{
    if (expr_->variable())
        return get_subvariable(expr_->variable(), index_);
    return nullptr;
}

ValuePtr IndexingExpression::evaluate_impl() const
{
    if (expr_->variable() == nullptr or runtime_.scope().is_variable_inline(expr_->variable()))
        return expr_->evaluate()->subvalue(index_);
    else
        return runtime_.serializer().write_node_def_input(variable());
}

void IndexingExpression::assign_impl(const ValuePtr& value)
{
    if (expr_->variable() == nullptr or runtime_.scope().is_variable_inline(expr_->variable()))
        expr_->evaluate()->set_subvalue(index_, value);
    else
        runtime_.serializer().write_node_def_output(variable(), value);
}
