//
// Created by jaket on 08/07/2026.
//

#ifndef MXSLC_VALUES_INTERFACE_H
#define MXSLC_VALUES_INTERFACE_H

#include "common.h"

namespace mxslc::values
{
    template<typename T, typename... Args>
    shared_ptr<T> create_value(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }
}

#endif //MXSLC_VALUES_INTERFACE_H
