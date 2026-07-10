//
// Created by jaket on 08/07/2026.
//

#ifndef MXSLC_VALUES_INTERFACE_H
#define MXSLC_VALUES_INTERFACE_H

#include "common.h"
#include "primitive_t.h"

namespace mxslc::serialize::values
{
#define TYPE_DEF(T) class T; \
using T##Ptr = shared_ptr<T>;

    TYPE_DEF(Value)
    TYPE_DEF(BasicValue)
    TYPE_DEF(InterfaceValue)
    TYPE_DEF(NodeGraphOutputValue)
    TYPE_DEF(NodeGraphValue)
    TYPE_DEF(NodeOutputValue)
    TYPE_DEF(NodeOutputGraphValue)
    TYPE_DEF(NodeValue)

#undef TYPE_DEF

    BasicValuePtr create_value(primitive_t value);

    template<typename T, typename... Args>
    shared_ptr<T> create_value(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    template<typename T>
    BasicValuePtr create_value()
    {
        return create_value(primitive_t{T{}});
    }

    template<typename T>
    shared_ptr<T> cast_value(const ValuePtr& value)
    {
        return std::dynamic_pointer_cast<T>(value);
    }
}

#endif //MXSLC_VALUES_INTERFACE_H
