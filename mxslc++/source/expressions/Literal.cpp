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

void Literal::init_impl(const vector<TypeInfoPtr>& types)
{
    type_ = std::make_shared<TypeInfo>(value_);

    if (std::holds_alternative<int>(value_))
    {
        const TypeInfoPtr int_type = runtime_.scope().get_type(TypeInfo::Int);
        const TypeInfoPtr float_type = runtime_.scope().get_type(TypeInfo::Float);
        if (not int_type->is_in(types) and float_type->is_in(types))
        {
            type_ = float_type;
        }
    }

    if (std::holds_alternative<string>(value_))
    {
        const TypeInfoPtr str_type = runtime_.scope().get_type(TypeInfo::String);
        const TypeInfoPtr filename_type = runtime_.scope().get_type(TypeInfo::Filename);
        if (not str_type->is_in(types) and filename_type->is_in(types))
        {
            type_ = filename_type;
        }
    }
}

TypeInfoPtr Literal::type_impl() const
{
    return type_;
}

ValuePtr Literal::evaluate_impl() const
{
    return std::make_shared<BasicValue>(value_, type_);
}
