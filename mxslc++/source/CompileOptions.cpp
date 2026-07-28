//
// Created by jaket on 26/07/2026.
//

#include "CompileOptions.h"

#include "utils/container_utils.h"
#include "utils/io_utils.h"
#include "utils/Logger.h"

namespace mxslc
{
    void CompileOptions::add_search_directory(fs::path dir)
    {
        if (not fs::is_directory(dir))
            Logger::warning("Search directory does not exist: " + dir.string());

        search_directories_.push_back(std::move(dir));
    }

    void CompileOptions::add_default_search_directories(const optional<fs::path>& src_path)
    {
        container_utils::extend(search_directories_, io_utils::get_default_search_directories(src_path));
    }
}
