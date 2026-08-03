//
// Created by jaket on 31/05/2026.
//

#include "runtime/utils/type_cast.h"

#include <cassert>

#include "utils/string_utils.h"
#include "runtime/interface.h"
#include "runtime/utils/invoke.h"
#include "runtime/Type.h"
#include "runtime/variables/Variable.h"

namespace mxslc::runtime_utils
{
    VarPtr type_cast(const TypePtr& type, const VarPtr& value, const bool force)
    {
        if (value->type()->equals(type, force))
            return value;

        assert(value->type()->is_compatible(type));

        if (value->child_count() == type->field_count())
            return create_variable(type, value);

        VarPtr cast_value;
        if (type->is_vector())
        {
            cast_value = invoke_constructor(type, value->children());
        }
        else
        {
            cast_value = invoke_function(type, "separate" + std::to_string(type->field_count()), value);
        }

        assert(cast_value->type()->equals(type));
        return cast_value;
    }
}
