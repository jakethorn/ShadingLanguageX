//
// Created by jaket on 27/11/2025.
//

#include "expressions/Literal.h"

#include "expressions/interface.h"
#include "runtime/interface.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"
#include "serialize/values/interface.h"
#include "serialize/values/StringValue.h"

namespace mxslc::expressions
{
    ExprPtr Literal::monomorphize(const TypePtr& template_type) const
    {
        return create_expression<Literal>(token_);
    }

    void Literal::init_impl(const vector<TypePtr>& types)
    {
        if (value_.is_a<int>() and not Type::Int->is_in(types))
        {
            // implicit cast from int to float
            if (Type::Float->is_in(types))
            {
                value_ = value_.cast<float>();
                return;
            }

            // implicit cast from int to bool
            if (Type::Bool->is_in(types))
            {
                value_ = value_.cast<bool>();
                return;
            }
        }

        if (value_.is_a<string>() and not Type::String->is_in(types))
        {
            // implicit cast from string to filename
            if (Type::Filename->is_in(types))
            {
                value_ = value_.cast<fs::path>();
                return;
            }

            // implicit cast from empty string to DF/shader types
            if (value_ == "")
            {
                const vector df_shader_types{
                    scope().get_type("material"),
                    scope().get_type("surfaceshader"),
                    scope().get_type("displacementshader"),
                    scope().get_type("volumeshader"),
                    scope().get_type("lightshader"),
                    scope().get_type("BSDF"),
                    scope().get_type("VDF"),
                    scope().get_type("EDF"),
                };

                if (const optional<TypePtr> target_type = container_utils::contains_one(types, df_shader_types))
                {
                    default_value_ = create_value<StringValue>("", *target_type);
                    return;
                }
            }
        }
    }

    TypePtr Literal::type_impl() const
    {
        if (default_value_)
            return default_value_->type();
        return value_.type();
    }

    VarPtr Literal::evaluate_impl() const
    {
        if (default_value_)
            return create_variable(default_value_);
        return create_variable(value_);
    }

    string Literal::to_string() const
    {
        return value_.to_string();
    }
}
