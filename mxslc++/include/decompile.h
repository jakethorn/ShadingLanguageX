//
// Created by jaket on 19/06/2026.
//

#ifndef MXSLC_DECOMPILE_H
#define MXSLC_DECOMPILE_H

#include <string>
#include <filesystem>
#include <optional>

#include <MaterialXCore/Document.h>

namespace mxslc
{
    std::string decompile_to_string(const std::filesystem::path& src_path);
    std::string decompile_to_string(const std::string& source);
    std::string decompile_to_string(const MaterialX::DocumentPtr& document);

    std::filesystem::path decompile_to_file(const std::filesystem::path& src_path, const std::optional<std::filesystem::path>& dst_path = std::nullopt);
    std::filesystem::path decompile_to_file(const std::filesystem::path& src_path, const std::filesystem::path& dst_path);
    std::filesystem::path decompile_to_file(const std::string& source, const std::filesystem::path& dst_path);
    std::filesystem::path decompile_to_file(const MaterialX::DocumentPtr& document, const std::filesystem::path& dst_path);
}

#endif //MXSLC_DECOMPILE_H
