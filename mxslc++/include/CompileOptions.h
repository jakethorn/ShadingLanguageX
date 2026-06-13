//
// Created by jaket on 27/05/2026.
//

#ifndef MXSLC_COMPILEOPTIONS_H
#define MXSLC_COMPILEOPTIONS_H

#include <optional>
#include <filesystem>
#include <string>

#include "primitive_t.h"

namespace mxslc
{
    struct CompileOptions
    {
        std::optional<std::filesystem::path> output_file = std::nullopt;
        std::string version{"1.39.5"};
        std::optional<std::string> func_name = std::nullopt;
        std::vector<primitive_t> func_args{};
        bool reduce_graph{true};

        bool has_function() const { return func_name.has_value(); }
    };
}

#endif //MXSLC_COMPILEOPTIONS_H
