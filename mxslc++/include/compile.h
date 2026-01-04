//
// Created by jaket on 02/01/2026.
//

#ifndef MXSLC_COMPILE_H
#define MXSLC_COMPILE_H

#include <filesystem>

namespace mxslc
{
    void compile(const std::filesystem::path& src_path, const std::filesystem::path& dst_path);
    void compile(const std::filesystem::path& src_path);
}

#endif //MXSLC_COMPILE_H
