//
// Created by jaket on 02/11/2025.
//

#include "utils/io_utils.h"

#include <fstream>
#include <sstream>

#if defined(_WIN32)
    #include <windows.h>
#elif defined(__APPLE__)
    #include <mach-o/dyld.h>
    #include <limits.h>
    #include <dlfcn.h>
    #include <stdlib.h>
#else
    #include <unistd.h>
    #include <limits.h>
    #include <dlfcn.h>
#endif

#include "errors/CompileError.h"

namespace mxslc::io_utils
{
    using std::ifstream;
    using std::ofstream;
    using std::ostringstream;

    string read_file(const fs::path& src_path)
    {
        ifstream file{src_path};

        if (not file.is_open())
        {
            throw CompileError{"Cannot read file: " + src_path.string()};
        }

        ostringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    void save_file(const fs::path& dst_path, const string& text)
    {
        ofstream file{dst_path};

        if (not file.is_open())
        {
            throw CompileError{"Cannot save file: " + dst_path.string()};
        }

        file << text;
    }

    fs::path get_executable_directory()
    {
#if defined(_WIN32)
        wchar_t path[MAX_PATH];
        const DWORD count = GetModuleFileNameW(NULL, path, MAX_PATH);
        if (count == 0 || count == MAX_PATH)
            throw CompileError{"Cannot determine executable path"};
        return fs::path{path}.parent_path();
#elif defined(__APPLE__)
        char path[PATH_MAX];
        uint32_t size = sizeof(path);
        if (_NSGetExecutablePath(path, &size) != 0)
            throw CompileError{"Cannot determine executable path: buffer too small"};
        // Resolve symlinks to get the real path
        char real_path[PATH_MAX];
        if (realpath(path, real_path) == nullptr)
            throw CompileError{"Cannot resolve executable path"};
        return fs::path{real_path}.parent_path();
#elif defined(__EMSCRIPTEN__)
        // Emscripten runs on a virtual filesystem rooted at '/', and the
        // libraries folder is preloaded there, so the current working
        // directory is the most useful search location.
        return fs::current_path();
#else
        // Linux and other Unix-like systems
        char path[PATH_MAX];
        const ssize_t count = readlink("/proc/self/exe", path, PATH_MAX - 1);
        if (count == -1)
            throw CompileError{"Cannot determine executable path"};
        path[count] = '\0';
        return fs::path{path}.parent_path();
#endif
    }

    fs::path get_python_module_directory()
    {
#ifdef _WIN32
        HMODULE module = nullptr;

        const BOOL ok = GetModuleHandleExW(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCWSTR>(&get_python_module_directory),
            &module
        );

        if (!ok || module == nullptr)
            throw CompileError{"Cannot determine python module path"};

        wchar_t path[MAX_PATH];
        const DWORD count = GetModuleFileNameW(module, path, MAX_PATH);
        if (count == 0 || count == MAX_PATH)
            throw CompileError{"Cannot determine python module path"};
        return fs::path{path}.parent_path();
#elif defined(__EMSCRIPTEN__)
        // Same as get_executable_directory(): there is no host file system to
        // query in the browser, so fall back to the current working directory.
        return fs::current_path();
#else
        Dl_info info;

        if (dladdr(reinterpret_cast<void*>(&get_python_module_directory), &info) == 0 || info.dli_fname == nullptr)
            throw CompileError{"Cannot determine python module path"};

        return fs::path{info.dli_fname}.parent_path();
#endif
    }

    vector<fs::path> get_default_search_directories(const optional<fs::path>& src_path)
    {
        vector<fs::path> dirs;
        if (src_path)
            dirs.push_back(src_path->parent_path());
        dirs.push_back(fs::current_path());
        dirs.push_back(get_python_module_directory());
        dirs.push_back(get_executable_directory());
        return dirs;
    }

    void search(const vector<fs::path>& search_dirs, const fs::path& path, const std::function<void(const fs::path&)>& on_found)
    {
        if (fs::is_regular_file(path))
        {
            on_found(path);
            return;
        }

        if (path.is_absolute())
            throw CompileError{"File " + path.string() + " could not be found."};

        string searched_paths = path.string() + "\n";

        for (const fs::path& dir : search_dirs)
        {
            fs::path full_path = dir / path;
            if (fs::is_regular_file(full_path))
            {
                on_found(full_path);
                return;
            }

            searched_paths += full_path.string() + "\n";
        }

        throw CompileError{"File " + path.string() + " could not be found.\nSearched paths:\n" + searched_paths};
    }
}
