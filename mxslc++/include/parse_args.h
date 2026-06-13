//
// Created by jaket on 03/01/2026.
//

#ifndef MXSLC_PARSE_ARGS_H
#define MXSLC_PARSE_ARGS_H

#include <filesystem>
#include <string>
#include <vector>

#include "CompileOptions.h"

namespace mxslc
{
    struct CommandLineArgs
    {
        std::filesystem::path input_file;
        CompileOptions options;
        bool is_valid = true;

        static CommandLineArgs invalid();
    };

    CommandLineArgs parse_args(int argc, char* argv[]);
    CommandLineArgs parse_args(const std::vector<std::string>& argv);
    CommandLineArgs parse_args(const std::filesystem::path& response_path);
}

#endif //MXSLC_PARSE_ARGS_H
