//
// Created by jaket on 07/07/2026.
//

#ifndef MXSLC_ACCESSORS_INTERFACE_H
#define MXSLC_ACCESSORS_INTERFACE_H

#include "common.h"

namespace mxslc::expressions
{
    template<typename T, typename... Args>
    unique_ptr<T> create_accessor(Args&&... types)
    {
        return std::make_unique<T>(std::forward<Args>(types)...);
    }

#define TYPE_DEF(T) class T; \
using T##Ptr = unique_ptr<T>;

    TYPE_DEF(Accessor)
    TYPE_DEF(ComponentAccessor)
    TYPE_DEF(FieldAccessor)
    TYPE_DEF(PortAccessor)

#undef TYPE_DEF
}

#endif //MXSLC_ACCESSORS_INTERFACE_H
