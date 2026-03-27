//
// Created by jaket on 03/12/2025.
//

#include "Variable.h"
#include "CompileError.h"
#include "TypeInfo.h"
#include "values/Value.h"

Variable::Variable(ModifierList mods, Token name, ValuePtr val)
    : mods_{std::move(mods)},
    name_{std::move(name)},
    val_{std::move(val)}
{
    mods_.validate("const"s, "mutable"s, "global"s);
    if (is_const() and is_mutable())
        throw CompileError{name_, "Variables cannot be both const and mutable"s};
}

TypeInfoPtr Variable::type() const
{
    return val_->type();
}
