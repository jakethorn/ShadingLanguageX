//
// Created by jaket on 27/05/2026.
//

#ifndef MXSLC_COMPILEOPTIONS_H
#define MXSLC_COMPILEOPTIONS_H

#include <optional>
#include <filesystem>
#include <string>

#include "constants.h"
#include "primitive_t.h"
#include "Variable.h"

namespace mxslc
{
    struct CompileOptions
    {
        std::optional<std::filesystem::path> output_file = std::nullopt;
        std::string version{DEFAULT_MTLX_VERSION};
        std::optional<std::string> func_name = std::nullopt;
        std::vector<primitive_t> func_args{};
        std::vector<Variable> globals{};
        bool error_on_missing_globals{true};
        bool error_on_unused_globals{true};
        bool reduce_graph{true};

        bool has_function() const { return func_name.has_value(); }
    };
}

#endif //MXSLC_COMPILEOPTIONS_H
