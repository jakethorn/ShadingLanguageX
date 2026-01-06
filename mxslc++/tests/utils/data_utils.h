//
// Created by jaket on 03/01/2026.
//

#ifndef FENNEC_DATA_UTILS_H
#define FENNEC_DATA_UTILS_H

#include <string>
#include <filesystem>
#include <fstream>

using std::string;
namespace fs = std::filesystem;

inline fs::path get_test_data_dir()
{
    return fs::current_path() / "tests" / "data";
}

inline fs::path get_test_data(const string& relative_filepath)
{
    return get_test_data_dir() / relative_filepath;
}

inline string read_file(const fs::path& filepath)
{
    const std::ifstream file{filepath};
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

#endif //FENNEC_DATA_UTILS_H
