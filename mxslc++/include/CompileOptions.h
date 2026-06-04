//
// Created by jaket on 27/05/2026.
//

#ifndef MXSLC_COMPILEOPTIONS_H
#define MXSLC_COMPILEOPTIONS_H

#include <optional>
#include <filesystem>
#include <string>

namespace mxslc
{
    struct CompileOptions
    {
        std::optional<std::filesystem::path> output_file = std::nullopt;
        std::string version = "1.39.5";
        bool reduce_graph = true;
    };
}

#endif //MXSLC_COMPILEOPTIONS_H
