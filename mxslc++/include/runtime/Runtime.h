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

        static Runtime& create(CompileOptions opts);
        static Runtime& get();

        VarPtr global(const string& name) const;
        vector<fs::path> include_directories() const { return opts_.search_directories(); }

        mx::DocumentPtr materialx_library() { return mtlx_lib_; }

        Scope& scope();
        void enter_scope(string name = "");
        void enter_scope(FuncPtr func);
        void exit_scope();

        Serializer& serializer();

        void destroy() const;

    private:
        void load_libraries();
        void load_materialx_library();

        CompileOptions opts_;
        mx::DocumentPtr mtlx_lib_;
        ScopePtr scope_;
        Serializer serializer_;

        mutable vector<string> used_globals;

        static unique_ptr<Runtime> instance_;
    };
}

#endif //FENNEC_RUNTIME_H
