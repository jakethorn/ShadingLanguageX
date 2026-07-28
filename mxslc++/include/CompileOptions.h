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

        unordered_set<Macro> macros;

        unordered_map<string, VarPtr> globals;
        bool error_on_missing_globals{true};
        bool error_on_unused_globals{true};

        optional<string> func_name;
        vector<Primitive> func_args;

    public:
        CompileOptions() = default;

        void add_search_directory(fs::path dir);
        void add_default_search_directories(const optional<fs::path>& src_path = std::nullopt);
        const vector<fs::path>& search_directories() const { return search_directories_; }

        bool has_output_file() const { return output_file.has_value(); }
        bool has_function() const { return func_name.has_value(); }

    private:
        vector<fs::path> search_directories_;
    };
}

#endif //MXSLC_COMPILEOPTIONS_H
