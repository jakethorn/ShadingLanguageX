//
// Created by jaket on 02/11/2025.
//

#include "io_utils.h"

#if defined(__APPLE__)
#include <mach-o/dyld.h>  // For _NSGetExecutablePath
#include <limits.h>       // For PATH_MAX
#endif

#include <fstream>
#include <sstream>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <limits.h>
    #include <dlfcn.h>
#endif

#include "CompileError.h"

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

fs::path get_executable_dir()
{
#ifdef _WIN32
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
    return fs::path{path}.parent_path();
#elif defined(__APPLE__)
    char path[PATH_MAX];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) != 0) {
        throw CompileError{"Cannot determine executable path: buffer too small"s};
    }
    // Resolve symlinks to get the real path
    char real_path[PATH_MAX];
    if (realpath(path, real_path) == nullptr) {
        throw CompileError{"Cannot resolve executable path"s};
    }
    return fs::path{real_path}.parent_path();
#else
    // Linux and other Unix-like systems
    char path[PATH_MAX];
    const ssize_t count = readlink("/proc/self/exe", path, PATH_MAX - 1);
    if (count == -1)
        throw CompileError{"Cannot determine executable path"s};
    path[count] = '\0';
    return fs::path{path}.parent_path();
#endif
}

fs::path get_python_module_dir()
{
#ifdef _WIN32
    HMODULE module = nullptr;

    const BOOL ok = GetModuleHandleExW(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(&get_python_module_dir),
        &module
    );

    if (!ok || module == nullptr)
        throw CompileError{"Cannot determine python module path"s};

    wchar_t path[MAX_PATH];
    const DWORD count = GetModuleFileNameW(module, path, MAX_PATH);

    if (count == 0 || count == MAX_PATH)
        throw CompileError{"Cannot determine python module path"s};

    return fs::path{path}.parent_path();
#else
    Dl_info info;

    if (dladdr(reinterpret_cast<void*>(&get_python_module_dir), &info) == 0 || info.dli_fname == nullptr)
        throw CompileError{"Cannot determine python module path"s};

    return fs::path{info.dli_fname}.parent_path();
#endif
}

vector<fs::path> get_include_directories(const optional<fs::path>& src_path)
{
    vector<fs::path> dirs;
    if (src_path)
        dirs.push_back(src_path->parent_path());
    dirs.push_back(fs::current_path());
    dirs.push_back(get_python_module_dir());
    dirs.push_back(get_executable_dir());
    return dirs;
}
