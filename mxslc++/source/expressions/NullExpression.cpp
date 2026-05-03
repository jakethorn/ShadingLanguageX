//
// Created by jaket on 31/12/2025.
//

#include "NullExpression.h"

#include "CompileError.h"
#include "runtime/Variable.h"
#include "values/NullValue.h"

ExprPtr NullExpression::instantiate_template_types(const TypePtr& template_type) const
{
    return std::make_unique<NullExpression>(token_);
}

void NullExpression::init_impl(const vector<TypePtr>& types)
{
    if (types.size() != 1)
        throw CompileError{"Ambiguous null expression"s};
    type_ = types.at(0);
}

TypePtr NullExpression::type_impl() const
{
    return type_;
}

VarPtr NullExpression::evaluate_impl() const
{
    ValuePtr value = std::make_shared<NullValue>(type_);
    return Variable::create(std::move(value));
}
