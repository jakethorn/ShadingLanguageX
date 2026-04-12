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

SubVariable::SubVariable(VarPtr parent, const size_t index)
    : parent_{std::move(parent)}, index_{index}, name_{port_name(parent_->name(), index_)} { }

SubVariable::SubVariable(VarPtr parent, const string& name)
    : SubVariable{parent, parent->type()->field_index(name)} { }

bool SubVariable::is_const() const
{
    return field().is_const() or parent_->is_const();
}

bool SubVariable::is_mutable() const
{
    return field().is_mutable() or parent_->is_mutable();
}

bool SubVariable::is_global() const
{
    return parent_->is_global();
}

TypeInfoPtr SubVariable::type() const
{
    return field().type();
}

const FieldInfo& SubVariable::field() const
{
    return parent_->type()->field(index_);
}

const string& SubVariable::name() const
{
    return name_;
}

ValuePtr SubVariable::value() const
{
    return parent_->value()->subvalue(index_);
}

void SubVariable::set_value(const ValuePtr& val)
{
    parent_->value()->set_subvalue(index_, val);
}

VarPtr get_subvariable(VarPtr parent, size_t index)
{
    return std::make_shared<SubVariable>(std::move(parent), index);
}

VarPtr get_subvariable(VarPtr parent, const string& name)
{
    return std::make_shared<SubVariable>(std::move(parent), name);
}
