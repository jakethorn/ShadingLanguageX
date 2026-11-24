//
// Created by jaket on 03/06/2026.
//

#ifndef MXSLC_RUNTIMEAWARE_H
#define MXSLC_RUNTIMEAWARE_H

#include "common.h"

namespace mxslc
{
    struct CompileOptions;
    class ErrorReporter;
}

namespace mxslc::serialize
{
    class Serializer;
}

namespace mxslc::reflection
{
    class Reflection;
}

namespace mxslc::runtime
{
    class Runtime;
    class Scope;
}

namespace mxslc::runtime_utils
{
    using reflection::Reflection;

    class RuntimeAware
    {
    public:
        RuntimeAware() = default;
        virtual ~RuntimeAware() = default;

    protected:
        static const CompileOptions& compile_options();
        static Runtime& runtime();
        static Scope& scope();
        static Serializer& serializer();
        static Reflection& reflection();
        static ErrorReporter& error_reporter();
    };
}

#endif //MXSLC_RUNTIMEAWARE_H
