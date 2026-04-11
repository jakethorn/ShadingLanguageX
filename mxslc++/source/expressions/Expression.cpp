//
// Created by jaket on 21/11/2025.
//

#include "Expression.h"

#include <algorithm>
#include <cassert>

#include "CompileError.h"
#include "runtime/Runtime.h"
#include "runtime/TypeInfo.h"
#include "runtime/Variable.h"
#include "utils/str_utils.h"

#define TRY_START try {
#define TRY_END } catch (const CompileError& e) { throw CompileError{token_, e}; }

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
    TRY_START

    if (not try_init(types))
        throw CompileError{"Cannot initialize expression of type " + type_impl()->str() + " with " + TypeInfo::to_string(types)};

    TRY_END
}

bool Expression::try_init(const vector<TypeInfoPtr>& types)
{
    TRY_START

    for (const TypeInfoPtr& type : types)
        assert(type->is_resolved());

    init_subexpressions(types);
    init_impl(types);

    const TypeInfoPtr type = type_impl();
    assert(type->is_resolved());

    assigned_type_ = type->find_unique_compatible(types);
    is_initialized_ = assigned_type_ != nullptr || types.empty();

    return is_initialized_;

    TRY_END
}

TypeInfoPtr Expression::type() const
{
    TRY_START

    assert(is_initialized_);
    if (assigned_type_)
        return assigned_type_;
    TypeInfoPtr type = type_impl();
    assert(type->is_resolved());
    return type;

    TRY_END
}

ValuePtr Expression::evaluate() const
{
    TRY_START

    assert(is_initialized_);
    return evaluate_impl();

    TRY_END
}

void Expression::assign(const ValuePtr& value)
{
    TRY_START

    if (const VarPtr var = variable(); var != nullptr and (var->is_const() or not var->is_mutable()))
    {
        throw CompileError{"Const variable '" + var->name() + "' cannot be changed (use mutable modifier)"};
    }

    assert(is_initialized_);
    assign_impl(value);

    TRY_END
}

void Expression::assign_impl(const ValuePtr& value)
{
    throw CompileError{"This expression cannot be assigned to"s};
}

#undef TRY_START
#undef TRY_END
