//
// Created by jaket on 27/11/2025.
//

#include "Literal.h"
#include "runtime/Runtime.h"
#include "values/BasicValue.h"

ExprPtr Literal::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    return std::make_shared<Literal>(runtime_, token_);
}

TypeInfoPtr Literal::type_impl() const
{
    return runtime_.scope().get_type(token_.literal());
}

ValuePtr Literal::evaluate_impl() const
{
    return std::make_shared<BasicValue>(token_.literal());
}
