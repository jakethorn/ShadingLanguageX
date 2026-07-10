//
// Created by jaket on 03/06/2026.
//

#include "runtime/utils/RuntimeAware.h"
#include "runtime/Runtime.h"

namespace mxslc::runtime::runtime_utils
{
    Runtime& RuntimeAware::runtime()
    {
        return Runtime::get();
    }

    Scope& RuntimeAware::scope()
    {
        return Runtime::get().scope();
    }

    Serializer& RuntimeAware::serializer()
    {
        return Runtime::get().serializer();
    }
}
