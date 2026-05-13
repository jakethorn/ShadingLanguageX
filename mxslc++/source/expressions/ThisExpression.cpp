//
// Created by jaket on 06/05/2026.
//

#include "ThisExpression.h"

#include "runtime/Scope.h"
#include "runtime/Variable.h"

ExprPtr ThisExpression::instantiate_template_types(const TypePtr &template_type) const
{
    return std::make_shared<ThisExpression>(token_);
}

void ThisExpression::init_impl(const vector<TypePtr>& types)
{
    this_ = scope().get_variable("this"s);
}

TypePtr ThisExpression::type_impl() const
{
    return this_->type();
}

VarPtr ThisExpression::evaluate_impl() const
{
    return this_;
}
