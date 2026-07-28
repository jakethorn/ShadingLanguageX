//
// Created by jaket on 02/01/2026.
//

#ifndef FENNEC_RUNTIME_H
#define FENNEC_RUNTIME_H

#include <MaterialXCore/Document.h>

#include "common.h"
#include "CompileOptions.h"
#include "serialize/Serializer.h"

namespace mxslc
{
    struct CompileOptions;
}

namespace mxslc::runtime
{
    class Runtime
    {
    public:
        explicit Runtime(CompileOptions opts);
        Runtime(CompileOptions opts, ScopePtr scope, Serializer serializer);

        static Runtime& create(CompileOptions opts);
        static Runtime& get();

        VarPtr global(const string& name) const;
        const vector<fs::path>& include_directories() const { return opts_.search_directories; }

        void load_materialx_library(const string& version);
        mx::DocumentPtr materialx_library() { return mtlx_lib_; }

        Scope& scope();
        void enter_scope(string name = "");
        void exit_scope();

        Serializer& serializer();

        void destroy() const;

    private:
        CompileOptions opts_;
        mx::DocumentPtr mtlx_lib_;
        ScopePtr scope_;
        Serializer serializer_;

        mutable vector<string> used_globals;

        static unique_ptr<Runtime> instance_;
    };
}

#endif //FENNEC_RUNTIME_H
