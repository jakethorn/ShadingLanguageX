//
// Created by jaket on 26/03/2026.
//

#include "Value.h"

#include <cassert>

#include "runtime/Type.h"

Value::Value(TypePtr type) : type_{std::move(type)}
{
    assert(type_->is_primitive());
}
