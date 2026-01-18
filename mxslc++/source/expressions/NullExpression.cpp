//
// Created by jaket on 31/12/2025.
//

#include "NullExpression.h"

#include "CompileError.h"
#include "values/NullValue.h"

ExprPtr NullExpression::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    return std::make_unique<NullExpression>(runtime_, token_);
}

void NullExpression::init_impl(const vector<TypeInfoPtr>& types)
{
    if (types.size() != 1)
        throw CompileError{token_, "Ambiguous null expression"s};
    type_ = types.at(0);
}

TypeInfoPtr NullExpression::type_impl() const
{
    return type_;
}

ValuePtr NullExpression::evaluate_impl() const
{
    return std::make_shared<NullValue>(type_);
}
