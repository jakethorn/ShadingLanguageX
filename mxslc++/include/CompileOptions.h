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

        bool error_on_missing_globals{true};
        bool error_on_unused_globals{true};

        optional<string> func_name;

    public:
        CompileOptions() = default;

        void add_search_directory(fs::path dir);
        void add_default_search_directories(const optional<fs::path>& src_path = std::nullopt);
        void set_current_working_directory(fs::path dir);
        void clear_search_directories();
        vector<fs::path> search_directories() const;

        void add_macro(Macro macro);
        void add_macro(string macro);
        void add_macro(string name, const string& body);
        void remove_macro(const string& name);
        void clear_macros();
        const Macro& get_macro(const string& name) const;
        bool has_macro(const string& name) const;
        const unordered_map<string, Macro>& macros() const { return macros_; }

        void add_global(string name, VarPtr value);
        void add_global(string name, Primitive value);
        void set_globals(unordered_map<string, VarPtr> globals);
        void set_globals(const unordered_map<string, Primitive>& globals);
        void clear_globals();
        const VarPtr& get_global(const string& name) const;
        bool has_global(const string& name) const;
        const unordered_map<string, VarPtr>& globals() const { return globals_; }

        void add_entry_function_argument(VarPtr value);
        void add_entry_function_argument(Primitive value);
        void set_entry_function_arguments(vector<VarPtr> args);
        void set_entry_function_arguments(const vector<Primitive>& args);
        void clear_entry_function_arguments();
        const vector<VarPtr>& entry_function_arguments() const { return func_args_; }

        bool has_output_file() const { return output_file.has_value(); }
        bool has_entry_function() const { return func_name.has_value(); }

    private:
        optional<fs::path> cwd_;
        vector<fs::path> search_dirs_;
        unordered_map<string, Macro> macros_;
        unordered_map<string, VarPtr> globals_;
        vector<VarPtr> func_args_;
    };
}

#endif //MXSLC_COMPILEOPTIONS_H
