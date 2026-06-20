//
// Created by jaket on 19/06/2026.
//

#ifndef MXSLC_DECOMPILE_H
#define MXSLC_DECOMPILE_H

#include <filesystem>

namespace mxslc
{
    std::filesystem::path decompile_to_file(const std::filesystem::path& src_path);
}

#endif //MXSLC_DECOMPILE_H
