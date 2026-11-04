//
// Created by jaket on 02/01/2026.
//

#ifndef FENNEC_RUNTIME_H
#define FENNEC_RUNTIME_H

#include <MaterialXCore/Document.h>

#include "common.h"
#include "CompileOptions.h"
#include "serialize/Serializer.h"
#include "reflection/Reflection.h"

namespace mxslc
{
    struct CompileOptions;
}

namespace mxslc::runtime
{
    using reflection::Reflection;

    class Runtime
    {
    public:
        explicit Runtime(CompileOptions opts);

        static Runtime& create(CompileOptions opts);
        static Runtime& get();

        const CompileOptions& compile_options() const { return opts_; }
        mx::DocumentPtr materialx_library() { return mtlx_lib_; }

        VarPtr global(const string& name) const;

        Scope& scope();
        void enter_scope(string name = "");
        void enter_scope(FuncPtr func);
        void exit_scope();

        Serializer& serializer();
        Reflection& reflection();

        void finalise() const;

    private:
        void add_libraries();
        void add_materialx_library();
        void check_unused_globals() const;

        CompileOptions opts_;
        mx::DocumentPtr mtlx_lib_;
        ScopePtr scope_;
        Serializer serializer_;
        Reflection reflection_;

        mutable vector<string> used_globals;

        static unique_ptr<Runtime> instance_;
    };
}

#endif //FENNEC_RUNTIME_H
