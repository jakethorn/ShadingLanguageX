//
// Created by jaket on 26/03/2026.
//

#include "Value.h"
#include "runtime/TypeInfo.h"

ValuePtr Value::subvalue(const size_t i) const
{
    throw CompileError{"Cannot get subvalue"s};
}

ValuePtr Value::subvalue(const Token& name) const
{
    return subvalue(type_->field_index(name));
}

void Value::set_subvalue(size_t i, const ValuePtr& value)
{
    throw CompileError{"Cannot set subvalue"s};
}

void Value::set_subvalue(const Token& name, const ValuePtr& value)
{
    set_subvalue(type_->field_index(name), value);
}
