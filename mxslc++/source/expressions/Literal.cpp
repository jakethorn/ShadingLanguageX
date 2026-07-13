//
// Created by jaket on 27/11/2025.
//

#include "expressions/Literal.h"

#include "expressions/interface.h"
#include "runtime/interface.h"
#include "runtime/Type.h"

namespace mxslc::expressions
{
    ExprPtr Literal::monomorphize(const TypePtr& template_type) const
    {
        return create_expression<Literal>(token_);
    }

    void Literal::init_impl(const vector<TypePtr>& types)
    {
        // implicit cast from int to float
        if (value_.is_a<int>())
        {
            if (not Type::Int->is_in(types) and Type::Float->is_in(types))
            {
                value_ = value_.cast<float>();
            }
        }

        // implicit cast from string to filename
        if (value_.is_a<string>())
        {
            if (not Type::String->is_in(types) and Type::Filename->is_in(types))
            {
                value_ = value_.cast<fs::path>();
            }
        }
    }

    TypePtr Literal::type_impl() const
    {
        return value_.type();
    }

    VarPtr Literal::evaluate_impl() const
    {
        return create_variable(value_);
    }
}
