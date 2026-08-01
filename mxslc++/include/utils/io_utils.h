//
// Created by jaket on 02/11/2025.
//

#ifndef FENNEC_IO_UTILS_H
#define FENNEC_IO_UTILS_H

#include <functional>

#include "common.h"

namespace mxslc::io_utils
{
    string read_file(const fs::path& src_path);
    void save_file(const fs::path& dst_path, const string& text);

    fs::path get_executable_directory();
    fs::path get_python_module_directory();
    vector<fs::path> get_default_search_directories(const optional<fs::path>& src_path = std::nullopt);

    void search(const vector<fs::path>& search_dirs, const fs::path& path, const std::function<void(const fs::path&)>& on_found);
}

#endif //FENNEC_IO_UTILS_H
