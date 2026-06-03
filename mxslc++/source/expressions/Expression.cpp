//
// Created by jaket on 21/11/2025.
//

#include "Expression.h"

#include <cassert>

#include "CompileError.h"
#include "runtime/Runtime.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"
#include "runtime/Variable.h"
#include "utils/type_cast.h"

#define TRY_START try {
#define TRY_END } catch (const CompileError& e) { throw CompileError{token_, e}; }

Expression::Expression(Token token) : token_{std::move(token)} { }

void Expression::init()
{
    init(vector<TypePtr>{});
}

void Expression::init(const TypePtr& type)
{
    init(vector{type});
}

void Expression::init(const string& type_name)
{
    const TypePtr type = scope().get_type(type_name);
    init(type);
}

void Expression::init(const vector<TypePtr>& types)
{
    TRY_START

    if (not try_init(types))
    {
        const TypePtr type = type_impl();
        if (type)
            throw CompileError{"Attempting to assign an expression of type " + type->str() + " to a variable of type " + Type::to_string(types)};
        else
            throw CompileError{"Attempting to assign an invalid expression to a variable of type " + Type::to_string(types)};
    }

    TRY_END
}

bool Expression::try_init(const vector<TypePtr>& types)
{
    TRY_START

    for (const TypePtr& type : types)
        assert(type->is_resolved());

    init_subexpressions(types);
    init_impl(types);

    const TypePtr type = type_impl();
    assert(type->is_resolved());

    target_type_ = type->find_unique_compatible(types);

    // empty means any type
    is_initialized_ = target_type_ != nullptr || types.empty();
    return is_initialized_;

    TRY_END
}

TypePtr Expression::type() const
{
    TRY_START

    assert(is_initialized_);
    if (target_type_)
        return target_type_;
    TypePtr type = type_impl();
    assert(type->is_resolved());
    return type;

    TRY_END
}

VarPtr Expression::evaluate() const
{
    TRY_START

    assert(is_initialized_);
    VarPtr value = evaluate_impl();

    if (target_type_)
    {
        assert(value->type()->is_equal(type_impl()));
        return type_cast(target_type_, value);
    }

    return value;

    TRY_END
}

#undef TRY_START
#undef TRY_END
