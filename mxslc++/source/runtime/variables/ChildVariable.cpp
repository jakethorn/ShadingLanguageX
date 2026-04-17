//
// Created by jaket on 14/04/2026.
//

#include "ChildVariable.h"

#include "CompileError.h"
#include "mtlx/mtlx_utils.h"
#include "runtime/FieldInfo.h"
#include "runtime/Runtime.h"
#include "runtime/TypeInfo.h"
#include "values/Value.h"

/*
 * Child Variable
 */

ChildVariable::ChildVariable(const Runtime& runtime, VarPtr parent, const size_t index)
    : IVariable{runtime}, parent_{std::move(parent)}, index_{index}, name_{port_name(parent_->name(), index_)} { }

ChildVariable::ChildVariable(const Runtime& runtime, VarPtr parent, const string& name)
    : ChildVariable{runtime, parent, parent->type()->field_index(name)} { }

bool ChildVariable::is_const() const
{
    return field().is_const() or parent_->is_const();
}

bool ChildVariable::is_mutable() const
{
    return field().is_mutable() or parent_->is_mutable();
}

bool ChildVariable::is_global() const
{
    return parent_->is_global();
}

TypeInfoPtr ChildVariable::type() const
{
    return field().type();
}

const FieldInfo& ChildVariable::field() const
{
    return parent_->type()->field(index_);
}

const string& ChildVariable::name() const
{
    return name_;
}

ValuePtr ChildVariable::value()
{
    if (is_inline())
        return parent_->raw_value()->subvalue(index_);
    else
        return runtime_.serializer().write_node_def_input(self());
}

ValuePtr ChildVariable::subvalue(const size_t index)
{
    return child(index)->value();
}

void ChildVariable::set_value(ValuePtr value)
{
    validate(value);
    if (is_inline())
        parent_->raw_value()->set_subvalue(index_, std::move(value));
    else
        runtime_.serializer().write_node_def_output(self(), value);
}

void ChildVariable::set_subvalue(const size_t index, ValuePtr value)
{
    child(index)->set_value(std::move(value));
}

void ChildVariable::set_const(const bool cnt)
{
    field().set_const(cnt);
}

void ChildVariable::set_mutable(const bool mut)
{
    field().set_mutable(mut);
}

ValuePtr ChildVariable::raw_value() const
{
    return parent_->raw_value()->subvalue(index_);
}
