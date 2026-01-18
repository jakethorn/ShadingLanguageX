//
// Created by jaket on 21/11/2025.
//

#include "Expression.h"

#include <algorithm>
#include <cassert>

#include "CompileError.h"
#include "runtime/Runtime.h"
#include "utils/template_utils.h"

Expression::Expression(const Runtime& runtime) : runtime_{runtime} { }
Expression::Expression(const Runtime& runtime, Token token) : runtime_{runtime}, token_{std::move(token)} { }

void Expression::init()
{
    init(vector<TypeInfoPtr>{});
}

void Expression::init(const TypeInfoPtr& type)
{
    init(vector{type});
}

void Expression::init(const string& type_name)
{
    const TypeInfoPtr type = runtime_.scope().get_type(type_name);
    init(type);
}

void Expression::init(const vector<TypeInfoPtr>& types)
{
    if (not try_init(types))
        throw CompileError{token_, "Expression failed to initialize"s};
}

bool Expression::try_init(const vector<TypeInfoPtr>& types)
{
    if (not is_initialized_)
    {
        init_subexpressions(types);
        init_impl(types);
        is_initialized_ = types.empty() || contains(types, type_impl());
    }

    return is_initialized_;
}

TypeInfoPtr Expression::type() const
{
    assert(is_initialized_);
    return type_impl();
}

ValuePtr Expression::evaluate() const
{
    assert(is_initialized_);
    return evaluate_impl();
}
