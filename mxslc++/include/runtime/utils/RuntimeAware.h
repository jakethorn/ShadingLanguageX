//
// Created by jaket on 03/06/2026.
//

#ifndef MXSLC_RUNTIMEAWARE_H
#define MXSLC_RUNTIMEAWARE_H

#include "common.h"

namespace mxslc::serialize
{
    class Serializer;
}

namespace mxslc::runtime
{
    class Runtime;
    class Scope;
}

namespace mxslc::runtime::runtime_utils
{
    class RuntimeAware
    {
    public:
        RuntimeAware() = default;
        virtual ~RuntimeAware() = default;

    protected:
        static Runtime& runtime();
        static Scope& scope();
        static Serializer& serializer();
    };
}

#endif //MXSLC_RUNTIMEAWARE_H
