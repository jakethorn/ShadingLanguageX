//
// Created by jaket on 27/11/2025.
//

#include "expressions/Literal.h"

#include "expressions/interface.h"
#include "runtime/interface.h"
#include "runtime/Runtime.h"
#include "runtime/Type.h"
#include "serialize/values/BasicValue.h"
#include "serialize/values/interface.h"

namespace mxslc::expressions
{
    ExprPtr Literal::monomorphize(const TypePtr& template_type) const
    {
        return create_expression<Literal>(token_);
    }

    void Literal::init_impl(const vector<TypePtr>& types)
    {
        type_ = Type::of(value_);

        // implicit cast from int to float
        if (std::holds_alternative<int>(value_))
        {
            if (not Type::Int->is_in(types) and Type::Float->is_in(types))
            {
                value_ = static_cast<float>(std::get<int>(value_));
                type_ = Type::Float;
            }
        }

        // implicit cast from string to filename
        if (std::holds_alternative<string>(value_))
        {
            if (not Type::String->is_in(types) and Type::Filename->is_in(types))
            {
                type_ = Type::Filename;
            }
        }
    }

    TypePtr Literal::type_impl() const
    {
        return type_;
    }

    VarPtr Literal::evaluate_impl() const
    {
        BasicValuePtr value = create_value<BasicValue>(value_, type_);
        return create_variable(std::move(value));
    }
}
