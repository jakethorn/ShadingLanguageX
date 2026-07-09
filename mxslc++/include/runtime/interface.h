//
// Created by jaket on 07/07/2026.
//

#ifndef MXSLC_RUNTIME_INTERFACE_H
#define MXSLC_RUNTIME_INTERFACE_H

#include "common.h"
#include "runtime/Type.h"

namespace mxslc::runtime
{
    VarPtr create_variable();

    template<typename... Args>
    FuncPtr create_function(Args&&... args)
    {
        return std::make_shared<Function>(std::forward<Args>(args)...);
    }

    template<typename... Args>
    TypePtr create_type(Args&&... args)
    {
        return std::make_shared<Type>(std::forward<Args>(args)...);
    }
}

#endif //MXSLC_RUNTIME_INTERFACE_H
