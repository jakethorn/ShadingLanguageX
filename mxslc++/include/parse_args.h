//
// Created by jaket on 03/01/2026.
//

#ifndef MXSLC_PARSE_ARGS_H
#define MXSLC_PARSE_ARGS_H

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace mxslc
{
    struct CompileOptions
    {
        std::filesystem::path input_file;
        std::optional<std::filesystem::path> output_file = std::nullopt;
        std::optional<std::string> version = std::nullopt;
        bool is_valid = false;
    };

    CompileOptions parse_args(int argc, char* argv[]);
    CompileOptions parse_args(const std::vector<std::string>& argv);
}

#endif //MXSLC_PARSE_ARGS_H
