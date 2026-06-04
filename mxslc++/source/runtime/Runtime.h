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

    static Runtime& create(const optional<fs::path>& src_path, const string& version, bool reduce_graph);
    static Runtime& get();

    const vector<fs::path>& include_directories() { return include_dirs_; }

    void load_materialx_library(const string& version);
    mx::DocumentPtr materialx_library() { return mtlx_lib_; }

    Scope& scope();
    void enter_scope(string name = ""s);
    void exit_scope();

    MtlXSerializer& serializer();

private:
    vector<fs::path> include_dirs_;
    mx::DocumentPtr mtlx_lib_;
    ScopePtr scope_;
    MtlXSerializer serializer_;

    static unique_ptr<Runtime> instance_;
};

#endif //FENNEC_RUNTIME_H
