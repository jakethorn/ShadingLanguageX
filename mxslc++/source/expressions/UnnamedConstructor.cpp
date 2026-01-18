//
// Created by jaket on 11/01/2026.
//

#include "UnnamedConstructor.h"

#include "CompileError.h"
#include "runtime/TypeInfo.h"
#include "values/UnnamedStructValue.h"

ExprPtr UnnamedConstructor::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    vector<ExprPtr> instantiated;
    for (const ExprPtr& expr : exprs_)
        instantiated.push_back(expr->instantiate_template_types(template_type));
    return std::make_unique<UnnamedConstructor>(runtime_, std::move(instantiated));
}

void UnnamedConstructor::init_subexpressions(const vector<TypeInfoPtr>& types)
{
    if (exprs_.size() == 1)
    {
        exprs_.at(0)->init(types);
        return;
    }

    while (initialised_expr_count_ < exprs_.size())
    {
        const size_t prev_initialised_expr_count = initialised_expr_count_;
        try_init_expressions(types);

        if (initialised_expr_count_ == prev_initialised_expr_count)
            throw CompileError{token_, "Invalid constructor call"s};
    }
}

TypeInfoPtr UnnamedConstructor::type_impl() const
{
    vector<TypeInfoPtr> types;
    types.reserve(exprs_.size());
    for (const ExprPtr& expr : exprs_)
        types.push_back(expr->type());
    return TypeInfo::create_unnamed_struct_type(types);
}

ValuePtr UnnamedConstructor::evaluate_impl() const
{
    vector<ValuePtr> values;
    values.reserve(exprs_.size());
    for (const ExprPtr& expr : exprs_)
        values.push_back(expr->evaluate());
    return std::make_shared<UnnamedStructValue>(std::move(values));
}

void UnnamedConstructor::try_init_expressions(const vector<TypeInfoPtr>& types)
{
    for (size_t i = 0; i < exprs_.size(); ++i)
    {
        if (exprs_[i]->is_initialized())
            continue;
        if (exprs_[i]->try_init(index_types(types, i)))
            ++initialised_expr_count_;
    }
}

vector<TypeInfoPtr> UnnamedConstructor::index_types(const vector<TypeInfoPtr>& types, const size_t index) const
{
    vector<TypeInfoPtr> index_types;
    for (const TypeInfoPtr& type : types)
    {
        bool is_compatible = true;

        if (type->field_count() != exprs_.size())
        {
            is_compatible = false;
        }
        else
        {
            for (size_t i = 0; i < exprs_.size(); ++i)
            {
                if (exprs_[i]->is_initialized() and type->field_type(i) != exprs_[i]->type())
                    is_compatible = false;
            }
        }

        if (is_compatible)
            index_types.push_back(type->field_type(index));
    }

    return index_types;
}
