//
// Created by jaket on 21/11/2025.
//

#include "Expression.h"

#include <algorithm>
#include <cassert>

#include "CompileError.h"
#include "runtime/Runtime.h"
#include "utils/template_utils.h"

Expression::Expression(const Runtime &runtime) : runtime_{runtime} { }
Expression::Expression(const Runtime& runtime, Token token) : runtime_{runtime}, token_{std::move(token)} { }

void Expression::init()
{
    init(vector<Type>{});
}

void Expression::init(const Type& type)
{
    init(vector{type});
}

void Expression::init(const vector<Type>& types)
{
    if (not try_init(types))
        throw CompileError{token_, "Invalid expression type"};
}

bool Expression::try_init(const vector<Type>& types)
{
    if (not is_initialized_)
    {
        init_subexpressions(types);
        init_impl(types);
        if (types.empty())
            is_initialized_ = runtime_.scope().has_type(type_impl());
        else
            is_initialized_ = contains(types, type_impl());
    }

    return is_initialized_;
}

const Type& Expression::type() const
{
    assert(is_initialized_);
    return type_impl();
}

ValuePtr Expression::evaluate() const
{
    assert(is_initialized_);
    return evaluate_impl();
}
