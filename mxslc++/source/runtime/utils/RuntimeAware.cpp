//
// Created by jaket on 03/06/2026.
//

#include "../../../include/runtime/utils/RuntimeAware.h"
#include "runtime/Runtime.h"

namespace mxslc
{
    Runtime& RuntimeAware::runtime()
    {
        return Runtime::get();
    }

    Scope& RuntimeAware::scope()
    {
        return Runtime::get().scope();
    }

    MtlXSerializer& RuntimeAware::serializer()
    {
        return Runtime::get().serializer();
    }
}
