//
// Created by jaket on 08/07/2026.
//

#include "serialize/values/interface.h"

#include "Primitive.h"
#include "runtime/Variable.h"
#include "serialize/values/BasicValue.h"

namespace mxslc::serialize::values
{
    BasicValuePtr create_value(Primitive value)
    {
        return create_value<BasicValue>(std::move(value));
    }
}
