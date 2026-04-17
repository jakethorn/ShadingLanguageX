//
// Created by jaket on 25/03/2026.
//

#include "DotExpression.h"

#include "Identifier.h"
#include "runtime/Runtime.h"
#include "runtime/TypeInfo.h"
#include "runtime/variables/ChildVariable.h"
#include "values/Value.h"

ExprPtr DotExpression::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    ExprPtr expr = expr_->instantiate_template_types(template_type);
    return std::make_unique<DotExpression>(runtime_, std::move(expr), token_);
}

void DotExpression::init_subexpressions(const vector<TypeInfoPtr>& types)
{
    expr_->init();
}

TypeInfoPtr DotExpression::type_impl() const
{
    return expr_->type()->field_type(property());
}

VarPtr DotExpression::variable() const
{
    if (expr_->variable())
        return get_child_variable(expr_->variable(), property());
    return nullptr;
}

ValuePtr DotExpression::evaluate_impl() const
{
    if (expr_->variable() == nullptr or runtime_.scope().is_variable_inline(expr_->variable()))
        return expr_->evaluate()->subvalue(property());
    else
        return runtime_.serializer().write_node_def_input(variable());
}

void DotExpression::assign_impl(const ValuePtr& value)
{
    if (expr_->variable() == nullptr or runtime_.scope().is_variable_inline(expr_->variable()))
        expr_->evaluate()->set_subvalue(property(), value);
    else
        runtime_.serializer().write_node_def_output(variable(), value);
}
