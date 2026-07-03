//
// Created by jaket on 27/05/2026.
//

#ifndef MXSLC_COMPILEOPTIONS_H
#define MXSLC_COMPILEOPTIONS_H

#include <optional>
#include <filesystem>
#include <string>

#include "constants.h"
#include "Macro.h"
#include "primitive_t.h"
#include "Variable.h"

namespace mxslc
{
    struct CompileOptions
    {
        std::optional<std::filesystem::path> output_file = std::nullopt;

        std::string version{DEFAULT_MTLX_VERSION};
        bool reduce_graph{true};

        std::vector<std::filesystem::path> search_directories{};
        std::vector<std::filesystem::path> includes{};
        std::vector<std::filesystem::path> libraries{};

        std::vector<Macro> macros{};

        std::vector<Variable> globals{};
        bool error_on_missing_globals{true};
        bool error_on_unused_globals{true};

        std::optional<std::string> func_name = std::nullopt;
        std::vector<primitive_t> func_args{};

        bool has_output_file() const { return output_file.has_value(); }
        bool has_function() const { return func_name.has_value(); }
    };
}

#endif //MXSLC_COMPILEOPTIONS_H
