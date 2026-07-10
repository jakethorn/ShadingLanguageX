//
// Created by jaket on 31/05/2026.
//

#include "runtime/utils/type_cast.h"

#include <cassert>

#include "utils/string_utils.h"
#include "runtime/utils/invoke.h"
#include "runtime/Type.h"
#include "runtime/Variable.h"

namespace mxslc::runtime_utils
{
    using string_utils::str;

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
            cast_value = invoke_constructor(type, value->children());
        }
        else
        {
            cast_value = invoke_function(type, "separate" + str(type->field_count()), value);
        }

        assert(cast_value->type()->is_equal(type));
        return cast_value;
    }
}
