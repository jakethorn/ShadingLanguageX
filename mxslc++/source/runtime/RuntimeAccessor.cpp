//
// Created by jaket on 03/06/2026.
//

#include "RuntimeAccessor.h"
#include "Runtime.h"

Runtime& RuntimeAccessor::runtime()
{
    return Runtime::get();
}

Scope& RuntimeAccessor::scope()
{
    return Runtime::get().scope();
}

MtlXSerializer& RuntimeAccessor::serializer()
{
    return Runtime::get().serializer();
}
