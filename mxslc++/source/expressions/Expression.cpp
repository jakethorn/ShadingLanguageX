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
        if (error_message_.empty())
            throw CompileError{"Cannot initialize expression of type " + type_impl()->str() + " with " + Type::to_string(types)};
        else
            throw CompileError{error_message_};
    }
    TRY_END
}

bool Expression::try_init(const vector<TypePtr>& types)
{
    TRY_START

    for (const TypePtr& type : types)
        assert(type->is_resolved());

    try
    {
        init_subexpressions(types);
        init_impl(types);
    }
    catch (const CompileError& e)
    {
        error_message_ = e.what();
        return false;
    }

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

Scope& Expression::scope()
{
    return Runtime::get().scope();
}

MtlXSerializer& Expression::serializer()
{
    return Runtime::get().serializer();
}

#undef TRY_START
#undef TRY_END
