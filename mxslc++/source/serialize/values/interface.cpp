//
// Created by jaket on 08/07/2026.
//

#include "serialize/values/interface.h"

#include "runtime/Type.h"
#include "runtime/Variable.h"

namespace mxslc::serialize::values
{
    BasicValuePtr create_value(primitive_t value)
    {
        return create_value<BasicValue>(std::move(value));
    }
}
