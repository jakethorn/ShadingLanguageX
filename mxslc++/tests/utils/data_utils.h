//
// Created by jaket on 03/01/2026.
//

#ifndef FENNEC_DATA_UTILS_H
#define FENNEC_DATA_UTILS_H

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>

using std::string;
using std::vector;
namespace fs = std::filesystem;

using namespace std::string_literals;

constexpr bool overwrite_data_files()
{
    return false;
}

inline fs::path get_test_data_dir()
{
#ifdef TEST_ROOT
    return fs::path(TEST_ROOT) / "data";
#else
    return fs::current_path() / "tests" / "data";
#endif
}

inline fs::path get_test_data(const string& relative_filepath)
{
    return get_test_data_dir() / relative_filepath;
}

inline string read_file(const fs::path& src_path)
{
    const std::ifstream file{src_path};
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

inline void write_file(const fs::path& filepath, const string& text)
{
    std::ofstream file{filepath};
    file << text;
}

#endif //FENNEC_DATA_UTILS_H
