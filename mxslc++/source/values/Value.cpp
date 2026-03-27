//
// Created by jaket on 26/03/2026.
//

#include "Value.h"
#include "runtime/TypeInfo.h"

ValuePtr Value::subvalue(const size_t i) const
{
    throw CompileError("Value does not have subvalues"s);
}

ValuePtr Value::subvalue(const Token& name) const
{
    return subvalue(type_->field_index(name));
}

ValuePtr Value::cast(const TypeInfoPtr& type) const
{
    if (not type->is_compatible(type_))
        throw CompileError(type->name_token(), "Invalid cast"s);
    return cast_impl(type);
}
