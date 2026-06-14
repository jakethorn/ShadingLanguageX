//
// Created by jaket on 31/05/2026.
//

#include "type_cast.h"

#include <cassert>

#include "str_utils.h"
#include "expressions/FunctionCall.h"
#include "expressions/NamedConstructor.h"
#include "runtime/RuntimeUtils.h"
#include "runtime/Type.h"
#include "runtime/Variable.h"

VarPtr type_cast(const TypePtr& type, const VarPtr& value, const bool force)
{
    if (value->type()->is_equal(type, force))
        return value;

    assert(value->type()->is_compatible(type));

    if (value->child_count() == type->field_count())
        return Variable::create(type, value);

    VarPtr cast_value;
    if (type->is_vector())
    {
        cast_value = RuntimeUtils::invoke_constructor(type, value->children());
    }
    else
    {
        cast_value = RuntimeUtils::invoke_function("separate" + str(type->field_count()), value, type);
    }

    assert(cast_value->type()->is_equal(type));
    return cast_value;
}
