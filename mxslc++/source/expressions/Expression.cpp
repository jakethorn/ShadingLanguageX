//
// Created by jaket on 21/11/2025.
//

#include "Expression.h"

#include <cassert>

#include "CompileError.h"
#include "runtime/Runtime.h"
#include "runtime/Type.h"
#include "runtime/Variable.h"
#include "../errors/AmbiguousFunctionError.h"
#include "utils/type_cast.h"

#define TRY_START try {
#define TRY_END } catch (CompileError& e) { e.set_debug_info(token_); throw; }

Expression::Expression(Token token) : token_{std::move(token)} { }

void Expression::init()
{
    init(vector<TypePtr>{});
}

void Expression::init(const TypePtr& type)
{
    init(vector<TypePtr>{type});
}

void Expression::init(const vector<TypePtr>& types)
{
    TRY_START

    if (not try_init(types))
    {
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
    catch (const AmbiguousFunctionError& e)
    {
        error_message_ = e.what();
        is_initialized_ = false;
        return false;
    }
    
    if (types.empty() or (types.size() == 1 and types[0]->is_auto()))
    {
        is_initialized_ = true;
        return true;
    }

    const TypePtr type = type_impl();
    assert(type->is_resolved());

    target_type_ = type->find_unique_compatible(types);

    if (target_type_ == nullptr)
        error_message_ = "Attempting to assign an expression of type " + type->str() + " to a variable or parameter of type " + Type::to_string(types);

    is_initialized_ = target_type_ != nullptr;
    return is_initialized_;

    TRY_END
}

void Expression::reset()
{
    is_initialized_ = false;
    target_type_ = nullptr;
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
