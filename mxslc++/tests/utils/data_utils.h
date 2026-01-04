//
// Created by jaket on 03/01/2026.
//

#ifndef FENNEC_DATA_UTILS_H
#define FENNEC_DATA_UTILS_H

#include <string>
#include <filesystem>

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

#endif //FENNEC_DATA_UTILS_H
