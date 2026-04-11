//
// Created by jaket on 03/12/2025.
//

#include "Variable.h"
#include "CompileError.h"
#include "TypeInfo.h"
#include "mtlx/mtlx_utils.h"
#include "values/Value.h"

/*
 * Variable
 */

Variable::Variable(ModifierList mods, string name, ValuePtr val)
    : mods_{std::move(mods)},
    name_{std::move(name)},
    val_{std::move(val)}
{
    mods_.validate(TokenType::Const, TokenType::Mutable, TokenType::Global);
    if (is_const() and is_mutable())
        throw CompileError{"Variables cannot be both const and mutable"s};
}

TypeInfoPtr Variable::type() const
{
    return val_->type();
}

/*
 * SubVariable
 */

SubVariable::SubVariable(VarPtr owner, const size_t index)
    : owner_{std::move(owner)}, index_{index}, name_{port_name(owner_->name(), index_)} { }

SubVariable::SubVariable(VarPtr owner, const string& name)
    : SubVariable{owner, owner->type()->field_index(name)} { }

bool SubVariable::is_const() const
{
    return field().is_const() or owner_->is_const();
}

bool SubVariable::is_mutable() const
{
    return field().is_mutable() or owner_->is_mutable();
}

bool SubVariable::is_global() const
{
    return owner_->is_global();
}

TypeInfoPtr SubVariable::type() const
{
    return field().type();
}

const FieldInfo& SubVariable::field() const
{
    return owner_->type()->field(index_);
}

const string& SubVariable::name() const
{
    return name_;
}

ValuePtr SubVariable::value() const
{
    return owner_->value()->subvalue(index_);
}

void SubVariable::set_value(const ValuePtr& val)
{
    owner_->value()->set_subvalue(index_, val);
}

/*
 * Standalone Functions
 */

VarPtr get_subvariable(VarPtr owner, size_t index)
{
    return std::make_shared<SubVariable>(std::move(owner), index);
}

VarPtr get_subvariable(VarPtr owner, const string& name)
{
    return std::make_shared<SubVariable>(std::move(owner), name);
}
