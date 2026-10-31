//
// Created by jaket on 02/01/2026.
//

#ifndef FENNEC_RUNTIME_H
#define FENNEC_RUNTIME_H

#include <MaterialXCore/Document.h>

#include "common.h"
#include "CompileOptions.h"
#include "GlobalVariable.h"
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

        const CompileOptions& options() const { return opts_; }

        VarPtr global(const string& name) const;
        void declare_global(string name, TypePtr type, VarPtr value);
        const vector<GlobalVariable>& declared_globals() const { return declared_globals_; }

        mx::DocumentPtr materialx_library() { return mtlx_lib_; }

        Scope& scope();
        void enter_scope(string name = "");
        void enter_scope(FuncPtr func);
        void exit_scope();

        Serializer& serializer();

        void finalise() const;

    private:
        void add_libraries();
        void add_materialx_library();
        void check_unused_globals() const;

        CompileOptions opts_;
        mx::DocumentPtr mtlx_lib_;
        ScopePtr scope_;
        Serializer serializer_;

        mutable vector<string> used_globals;
        vector<GlobalVariable> declared_globals_;

        static unique_ptr<Runtime> instance_;
    };
}

#endif //FENNEC_RUNTIME_H
