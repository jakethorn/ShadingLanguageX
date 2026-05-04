//
// Created by jaket on 02/01/2026.
//

#ifndef FENNEC_RUNTIME_H
#define FENNEC_RUNTIME_H

#include <MaterialXCore/Document.h>

#include "utils/common.h"
#include "mtlx/MtlXSerializer.h"

class Runtime
{
public:
    Runtime();
    Runtime(ScopePtr scope, MtlXSerializer serializer);

    Scope& scope();
    MtlXSerializer& serializer();
    mx::DocumentPtr materialx_library() { return mtlx_lib_; }

    void enter_scope();
    void exit_scope();

    static Runtime& create(const string& version);
    static Runtime& get();

private:
    void load_materialx_library(const string& version);

    ScopePtr scope_;
    MtlXSerializer serializer_;
    mx::DocumentPtr mtlx_lib_;

    static unique_ptr<Runtime> instance_;
};

#endif //FENNEC_RUNTIME_H
