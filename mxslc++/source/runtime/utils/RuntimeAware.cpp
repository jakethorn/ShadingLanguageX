//
// Created by jaket on 03/06/2026.
//

#include "runtime/utils/RuntimeAware.h"
#include "runtime/Runtime.h"

namespace mxslc::runtime_utils
{
    const CompileOptions& RuntimeAware::compile_options()
    {
        return Runtime::get().compile_options();
    }

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

    Reflection& RuntimeAware::reflection()
    {
        return Runtime::get().reflection();
    }
}
