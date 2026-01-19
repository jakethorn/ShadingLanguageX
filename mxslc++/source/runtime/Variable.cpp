//
// Created by jaket on 03/12/2025.
//

#include "Variable.h"
#include "CompileError.h"
#include "TypeInfo.h"

Variable::Variable(ModifierList mods, TypeInfoPtr type, Token name)
    : Variable{std::move(mods), std::move(type), std::move(name), nullptr}
{

}

Variable::Variable(ModifierList mods, TypeInfoPtr type, Token name, ValuePtr value)
    : mods_{std::move(mods)},
    type_{std::move(type)},
    name_{std::move(name)},
    value_{std::move(value)}
{
    mods_.validate("const"s, "mutable"s, "global"s);
    if (is_const() and is_mutable())
        throw CompileError{name_, "Variables cannot be both const and mutable"s};
}
