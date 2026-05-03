//
// Created by jaket on 02/01/2026.
//

#ifndef FENNEC_RUNTIME_H
#define FENNEC_RUNTIME_H

#include "utils/common.h"
#include "mtlx/MtlXSerializer.h"

class Runtime
{
public:
    Runtime();
    Runtime(ScopePtr scope, MtlXSerializer serializer);

    Scope& scope();
    MtlXSerializer& serializer();

    void enter_scope();
    void exit_scope();

    static Runtime& create();
    static Runtime& get();

private:
    ScopePtr scope_;
    MtlXSerializer serializer_;

    static unique_ptr<Runtime> instance_;
};

#endif //FENNEC_RUNTIME_H
