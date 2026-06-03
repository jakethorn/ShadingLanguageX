//
// Created by jaket on 03/06/2026.
//

#ifndef MXSLC_RUNTIMEACCESSOR_H
#define MXSLC_RUNTIMEACCESSOR_H

class Runtime;
class Scope;
class MtlXSerializer;

class RuntimeAccessor
{
public:
    virtual ~RuntimeAccessor() = default;

protected:
    static Runtime& runtime();
    static Scope& scope();
    static MtlXSerializer& serializer();
};

#endif //MXSLC_RUNTIMEACCESSOR_H
