//
// Created by jaket on 27/11/2025.
//

#include "Literal.h"
#include "runtime/Runtime.h"
#include "runtime/Scope.h"
#include "runtime/TypeInfo.h"
#include "runtime/Variable.h"
#include "values/BasicValue.h"

ExprPtr Literal::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    return std::make_shared<Literal>(token_);
}

void Literal::init_impl(const vector<TypeInfoPtr>& types)
{
    type_ = std::make_shared<TypeInfo>(value_);

    // implicit cast from int to float
    if (std::holds_alternative<int>(value_))
    {
        const TypeInfoPtr int_type = scope().get_type(TypeInfo::Int);
        const TypeInfoPtr float_type = scope().get_type(TypeInfo::Float);
        if (not int_type->is_in(types) and float_type->is_in(types))
        {
            type_ = float_type;
        }
    }

    // implicit cast from string to filename
    if (std::holds_alternative<string>(value_))
    {
        const TypeInfoPtr string_type = scope().get_type(TypeInfo::String);
        const TypeInfoPtr filename_type = scope().get_type(TypeInfo::Filename);
        if (not string_type->is_in(types) and filename_type->is_in(types))
        {
            type_ = filename_type;
        }
    }
}

TypeInfoPtr Literal::type_impl() const
{
    return type_;
}

VarPtr Literal::evaluate_impl() const
{
    ValuePtr value = std::make_shared<BasicValue>(value_, type_);
    return Variable::create(std::move(value));
}
