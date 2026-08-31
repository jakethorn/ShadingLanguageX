//
// Created by jaket on 04/05/2026.
//

#include "expressions/accessors/FieldAccessor.h"

#include "expressions/Expression.h"
#include "runtime/Function.h"
#include "runtime/FunctionQuery.h"
#include "runtime/utils/invoke.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"
#include "runtime/utils/FunctionResolver.h"
#include "runtime/variables/Variable.h"

namespace mxslc::expressions
{
    FieldAccessor::FieldAccessor(ExprPtr value_expr, int field_index)
        : value_expr_{std::move(value_expr)}
    {
        const size_t field_count = value_expr_->type()->field_count();

        // allow negative indices
        if (field_index < 0)
            field_index = static_cast<int>(field_count) + field_index;

        if (field_index < 0 or field_index >= field_count)
            throw CompileError{"Index out of bounds"};

        field_ = field_index;
    }

    FieldAccessor::FieldAccessor(vector<TypePtr> target_types, ExprPtr value_expr, string field_name)
        : target_types_{std::move(target_types)}, value_expr_{std::move(value_expr)}
    {
        const bool has_field = value_expr_->type()->has_field(field_name);
        const bool has_method = scope().has_function({value_expr_->type(), target_types, field_name, /*is_parameterless*/true});

        if (not has_field and not has_method)
            throw CompileError{"Variable of type '" + value_expr_->type()->to_string() + "' does not have a field or parameterless method with the name: " + field_name};

        field_ = std::move(field_name);
    }

    TypePtr FieldAccessor::type() const
    {
        if (std::holds_alternative<int>(field_))
        {
            const int field_index = std::get<int>(field_);
            return value_expr_->type()->field_type(field_index);
        }
        else
        {
            const string field_name = std::get<string>(field_);

            if (value_expr_->type()->has_field(field_name))
                return value_expr_->type()->field_type(field_name);

            const FuncPtr method = runtime_utils::resolve_function({value_expr_->type(), target_types_, field_name, /*is_parameterless*/true});
            return method->return_type();
        }
    }

    VarPtr FieldAccessor::evaluate() const
    {
        VarPtr value = value_expr_->evaluate();

        if (std::holds_alternative<int>(field_))
        {
            const int field_index = std::get<int>(field_);
            return value->child(field_index);
        }
        else
        {
            const string field_name = std::get<string>(field_);

            if (value_expr_->type()->has_field(field_name))
                return value->child(field_name);

            return runtime_utils::invoke_method(target_types_, std::move(value), field_name);
        }
    }
}
