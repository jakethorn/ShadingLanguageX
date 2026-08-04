//
// Created by jaket on 08/07/2026.
//

#include "serialize/values/interface.h"

#include "Primitive.h"
#include "runtime/variables/Variable.h"
#include "serialize/values/CompileTimeValue.h"

namespace mxslc::serialize::values
{
    CompileTimeValuePtr create_value(Primitive value)
    {
        return create_value<CompileTimeValue>(std::move(value));
    }
}
