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

        search_dirs_.push_back(std::move(dir));
    }

    void CompileOptions::add_default_search_directories(const optional<fs::path>& src_path)
    {
        container_utils::extend(search_dirs_, io_utils::get_default_search_directories(src_path));
    }

    void CompileOptions::set_current_working_directory(fs::path dir)
    {
        cwd_ = std::move(dir);
    }

    vector<fs::path> CompileOptions::search_directories() const
    {
        vector<fs::path> dirs;
        if (cwd_)
            dirs.push_back(*cwd_);
        container_utils::extend(dirs, search_dirs_);
        return dirs;
    }

    void CompileOptions::add_macro(Macro macro)
    {
        macros_.insert_or_assign(macro.name(), std::move(macro));
    }

    void CompileOptions::remove_macro(const string& name)
    {
        macros_.erase(name);
    }

    const Macro& CompileOptions::get_macro(const string& name) const
    {
        if (has_macro(name))
            return macros_.at(name);
        throw CompileError{"Trying to access macro that does not exist with the name: " + name};
    }

    bool CompileOptions::has_macro(const string& name) const
    {
        return container_utils::contains(macros_, name);
    }
}
