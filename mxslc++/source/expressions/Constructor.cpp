//
// Created by jaket on 11/01/2026.
//

#include "Constructor.h"

#include "CompileError.h"
#include "values/UnnamedStructValue.h"

ExprPtr Constructor::instantiate_template_types(const Type& template_type) const
{
    vector<ExprPtr> instantiated;
    for (const ExprPtr& expr : exprs_)
        instantiated.push_back(expr->instantiate_template_types(template_type));
    return std::make_unique<Constructor>(runtime_, token_, std::move(instantiated));
}

void Constructor::init_subexpressions(const vector<Type>& types)
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

void Constructor::init_impl(const vector<Type>&)
{
    vector<Type> types;
    types.reserve(exprs_.size());
    for (const ExprPtr& expr : exprs_)
        types.push_back(expr->type());

    type_ = Type{std::move(types)};
}

const Type& Constructor::type_impl() const
{
    return type_;
}

ValuePtr Constructor::evaluate_impl() const
{
    vector<ValuePtr> values;
    values.reserve(exprs_.size());
    for (const ExprPtr& expr : exprs_)
        values.push_back(expr->evaluate());

    return std::make_shared<UnnamedStructValue>(std::move(values));
}

void Constructor::try_init_expressions(const vector<Type>& types)
{
    for (size_t i = 0; i < exprs_.size(); ++i)
    {
        if (not exprs_[i]->is_initialized())
        {
            if (const vector<Type> index_types = get_index_types(types, i);
                exprs_[i]->try_init(index_types))
            {
                ++initialised_expr_count_;
            }
        }
    }
}

vector<Type> Constructor::get_index_types(const vector<Type>& types, const size_t index) const
{
    vector<Type> index_types;
    for (const Type& type : types)
    {
        bool is_compatible = true;

        if (type.subtype_count() != exprs_.size())
        {
            is_compatible = false;
        }
        else
        {
            for (size_t i = 0; i < exprs_.size(); ++i)
            {
                if (exprs_[i]->is_initialized() and type.subtype(i) != exprs_[i]->type())
                    is_compatible = false;
            }
        }

        if (is_compatible)
            index_types.push_back(type.subtype(index));
    }

    return index_types;
}
