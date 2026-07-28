//
// Created by jaket on 27/05/2026.
//

#ifndef MXSLC_COMPILEOPTIONS_H
#define MXSLC_COMPILEOPTIONS_H

#include "common.h"
#include "constants.h"
#include "preprocess/Macro.h"
#include "Primitive.h"

namespace mxslc
{
    struct CompileOptions
    {
        optional<fs::path> output_file;

        string version{DEFAULT_MTLX_VERSION};
        bool reduce_graph{true};

        vector<fs::path> includes;
        vector<fs::path> libraries;


        unordered_map<string, VarPtr> globals;
        bool error_on_missing_globals{true};
        bool error_on_unused_globals{true};

        optional<string> func_name;
        vector<Primitive> func_args;

    public:
        CompileOptions() = default;

        void add_search_directory(fs::path dir);
        void add_default_search_directories(const optional<fs::path>& src_path = std::nullopt);
        void set_current_working_directory(fs::path dir);
        vector<fs::path> search_directories() const;

        void add_macro(Macro macro);
        void remove_macro(const string& name);
        const Macro& get_macro(const string& name) const;
        bool has_macro(const string& name) const;
        const unordered_map<string, Macro>& macros() const { return macros_; }

        bool has_output_file() const { return output_file.has_value(); }
        bool has_function() const { return func_name.has_value(); }

    private:
        optional<fs::path> cwd_;
        vector<fs::path> search_dirs_;
        unordered_map<string, Macro> macros_;
    };
}

#endif //MXSLC_COMPILEOPTIONS_H
