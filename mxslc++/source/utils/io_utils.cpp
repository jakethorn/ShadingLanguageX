//
// Created by jaket on 02/11/2025.
//

#include "io_utils.h"

#include <fstream>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <limits.h>
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
    return fs::path{path};
#else
    char path[PATH_MAX];
    const ssize_t count = readlink("/proc/self/exe", path, PATH_MAX - 1);
    if (count == -1)
        throw CompileError{"Cannot determine executable path"s};
    path[count] = '\0';
    return fs::path{path}.parent_path();
#endif
}
