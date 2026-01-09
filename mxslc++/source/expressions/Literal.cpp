//
// Created by jaket on 27/11/2025.
//

#include "Literal.h"

#include "runtime/values/BasicValue.h"

ExprPtr Literal::instantiate_template_types(const Type& template_type) const
{
    return std::make_unique<Literal>(runtime_, token_);
}

const Type& Literal::type_impl() const
{
    return type_;
}

ValuePtr Literal::evaluate_impl() const
{
    return std::make_shared<BasicValue>(value_);
}
