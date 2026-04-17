//
// Created by jaket on 03/12/2025.
//

#include "Variable.h"

#include <cassert>

#include "ChildVariable.h"
#include "CompileError.h"
#include "runtime/Runtime.h"
#include "runtime/TypeInfo.h"
#include "values/Value.h"

Variable::Variable(const Runtime& runtime, ModifierList mods, string name, ValuePtr value)
    : IVariable{runtime},
    mods_{std::move(mods)},
    name_{std::move(name)}
{
    mods_.validate(TokenType::Const, TokenType::Mutable, TokenType::Global);
    if (is_const() and is_mutable())
        throw CompileError{"Variables cannot be both const and mutable"s};

    set_value(std::move(value));
}

TypeInfoPtr Variable::type() const
{
    return value_->type();
}

ValuePtr Variable::value()
{
    if (is_inline())
        return value_;
    else
        return runtime_.serializer().write_node_def_input(self());
}

ValuePtr Variable::subvalue(const size_t index)
{
    return child(index)->value();
}

void Variable::set_value(ValuePtr value)
{
    validate(value);
    if (is_inline())
        value_ = std::move(value);
    else
        runtime_.serializer().write_node_def_output(self(), value);
}

void Variable::set_subvalue(const size_t index, ValuePtr value)
{
    child(index)->set_value(std::move(value));
}

void Variable::set_const(const bool cnt)
{
    if (cnt)
        mods_.add(TokenType::Const);
    else
        mods_.remove(TokenType::Const);
}

void Variable::set_mutable(const bool mut)
{
    if (mut)
        mods_.add(TokenType::Mutable);
    else
        mods_.remove(TokenType::Mutable);
}
