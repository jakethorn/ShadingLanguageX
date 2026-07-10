//
// Created by jaket on 03/01/2026.
//

#ifndef MXSLC_PARSE_ARGS_H
#define MXSLC_PARSE_ARGS_H

#include "common.h"
#include "CompileOptions.h"

namespace mxslc
{
    enum class Action
    {
        Unknown,
        Compile,
        Decompile
    };

    struct CommandLineArgs
    {
        Action action{Action::Compile};
        fs::path input_file;
        CompileOptions options;
        bool is_valid{true};

        static CommandLineArgs invalid();
    };

    CommandLineArgs parse_cli_args(int argc, char* argv[]);
    CommandLineArgs parse_cli_args(const vector<string>& argv);
    CommandLineArgs parse_cli_args(const fs::path& response_path);
}

#endif //MXSLC_PARSE_ARGS_H
