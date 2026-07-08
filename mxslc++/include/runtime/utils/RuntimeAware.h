//
// Created by jaket on 03/06/2026.
//

#ifndef MXSLC_RUNTIMEAWARE_H
#define MXSLC_RUNTIMEAWARE_H

namespace mxslc
{
    class MtlXSerializer;
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
        static MtlXSerializer& serializer();
    };
}

#endif //MXSLC_RUNTIMEAWARE_H
