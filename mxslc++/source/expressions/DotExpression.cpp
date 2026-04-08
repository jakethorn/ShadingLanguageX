//
// Created by jaket on 25/03/2026.
//

#include "DotExpression.h"

#include "CompileError.h"
#include "Identifier.h"
#include "values/BasicValue.h"
#include "runtime/Runtime.h"
#include "runtime/TypeInfo.h"
#include "runtime/Variable.h"

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
    return expr_->type()->field_type(token_);
}

VarPtr DotExpression::variable() const
{
    if (expr_->variable())
        return get_subvariable(expr_->variable(), token_);
    return nullptr;
}

ValuePtr DotExpression::evaluate_impl() const
{
    if (expr_->variable() == nullptr or runtime_.scope().is_variable_inline(expr_->variable()))
        return expr_->evaluate()->subvalue(token_);
    else
        return runtime_.serializer().write_node_def_input(variable());
}

void DotExpression::assign(const ValuePtr& value)
{
    if (expr_->variable() == nullptr or runtime_.scope().is_variable_inline(expr_->variable()))
        expr_->evaluate()->set_subvalue(token_, value);
    else
        runtime_.serializer().write_node_def_output(variable(), value);
}
