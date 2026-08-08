//
// Created by jaket on 11/01/2026.
//

#include "expressions/UnnamedConstructor.h"

#include "runtime/interface.h"
#include "runtime/Runtime.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"
#include "runtime/utils/monomorphize.h"
#include "errors/CompileError.h"
#include "expressions/interface.h"

namespace mxslc::expressions
{
    ExprPtr UnnamedConstructor::monomorphize(const TypePtr& template_type) const
    {
        vector<ExprPtr> exprs = runtime_utils::monomorphize(exprs_, template_type);
        return create_expression<UnnamedConstructor>(std::move(exprs), token_);
    }

    void UnnamedConstructor::init_subexpressions(const vector<TypePtr>& types)
    {
        if (expressions_are_initialized())
            return;

        while (initialized_expr_count_ < exprs_.size())
        {
            const size_t prev_initialized_expr_count = initialized_expr_count_;

            try_init_expressions(types);

            if (initialized_expr_count_ == prev_initialized_expr_count)
                throw CompileError{initialization_error_message(types)};
        }
    }

    TypePtr UnnamedConstructor::type_impl() const
    {
        vector<TypePtr> types;
        types.reserve(exprs_.size());
        for (const ExprPtr& expr : exprs_)
            types.push_back(expr->type());
        const TypePtr type = create_type(std::move(types));
        return scope().resolve_type(type);
    }

    VarPtr UnnamedConstructor::evaluate_impl() const
    {
        vector<VarPtr> values;
        values.reserve(exprs_.size());
        for (const ExprPtr& expr : exprs_)
            values.push_back(expr->evaluate());
        return create_variable(values);
    }

    bool UnnamedConstructor::expressions_are_initialized()
    {
        bool result = true;

        initialized_expr_count_ = 0;
        for (const ExprPtr& expr : exprs_)
        {
            if (expr->is_initialized())
            {
                expr->update();
                ++initialized_expr_count_;
            }
            else
            {
                result = false;
            }
        }

        return result;
    }

    void UnnamedConstructor::try_init_expressions(const vector<TypePtr>& types)
    {
        for (size_t i = 0; i < exprs_.size(); ++i)
        {
            if (exprs_[i]->is_initialized())
                continue;
            if (exprs_[i]->try_init(types_at_index(types, i)))
                ++initialized_expr_count_;
        }
    }

    namespace
    {
        vector<TypePtr> with_compatible_types(const vector<TypePtr>& types)
        {
            vector<TypePtr> result;
            for (const TypePtr& type : types)
            {
                result.push_back(type);

                if (type->is_vector())
                {
                    result.push_back(
                        Type::unnamed_struct(Type::Float, type->component_count())
                    );
                }
            }

            return result;
        }
    }

    vector<TypePtr> UnnamedConstructor::types_at_index(const vector<TypePtr>& types, const size_t index) const
    {
        vector<TypePtr> subtypes;

        for (const TypePtr& type : with_compatible_types(types))
        {
            if (type->field_count() == exprs_.size())
            {
                bool is_compatible = true;
                for (size_t i = 0; i < exprs_.size(); ++i)
                {
                    const TypePtr expr_type = exprs_[i]->type();
                    const TypePtr field_type = type->field_type(i);
                    if (exprs_[i]->is_initialized() and not expr_type->is_compatible_with(field_type))
                        is_compatible = false;
                }

                if (is_compatible)
                    subtypes.push_back(type->field_type(index));
            }

            if (type->is_tuple())
                subtypes.push_back(type->template_type());
        }

        return subtypes;
    }

    string UnnamedConstructor::initialization_error_message(const vector<TypePtr>& types) const
    {
        string message = "Cannot assign the unnamed constructor to a variable or parameter of type " + type_utils::to_string(types);
        message += "\nUnderlying errors:\n";

        for (size_t i = 0; i < exprs_.size(); ++i)
        {
            if (exprs_[i]->has_error())
                message += "Argument " + std::to_string(i) + ": " + exprs_[i]->error_message() + "\n";
        }

        return message;
    }

    string UnnamedConstructor::to_string() const
    {
        return "{" + join(exprs_, ", ") + "}";
    }
}
