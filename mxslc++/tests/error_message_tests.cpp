//
// Created by jaket on 10/04/2026.
//

#include "gtest/gtest.h"
#include <filesystem>
#include <string>
#include <vector>
#include "compile.h"
#include "errors/CompileError.h"
#include "utils/comp_utils.h"
#include "utils/data_utils.h"

namespace fs = std::filesystem;
using namespace std::string_literals;
using std::string;
using std::vector;

using error_message_tests = testing::TestWithParam<fs::path>;

TEST_P(error_message_tests, error_message_matches_groundtruth)
{
    const fs::path& input_path = GetParam();
    fs::path expected_path = input_path;
    expected_path.replace_extension(".txt");

    string actual_output;
    EXPECT_THROW(
        {
            try
            {
                mxslc::compile_to_string(input_path);
            }
            catch(const mxslc::CompileError& e)
            {
                actual_output = e.what();
                std::cout << "Caught expected error: " << e.what() << std::endl;
                throw;
            }
        },
        mxslc::CompileError
    ) << "\nFailed to throw CompileError: " << input_path;

    if constexpr (overwrite_data_files())
        write_file(expected_path, actual_output);

    const string expected_output = read_file(expected_path);
    const bool passed = actual_output == expected_output;

    EXPECT_TRUE(passed);
    if (not passed)
        print_debug_info(input_path, actual_output, expected_output);
}

vector<fs::path> get_error_message_files()
{
    const fs::path test_dir = get_test_data("error_message");

    if (not fs::exists(test_dir))
        return {};

    vector<fs::path> files;
    for (const auto& p : fs::recursive_directory_iterator(test_dir))
        if (p.path().extension() == ".mxsl")
            files.push_back(p.path());

    return files;
}

INSTANTIATE_TEST_SUITE_P(
    compiler,
    error_message_tests,
    testing::ValuesIn(get_error_message_files()),
    [](const testing::TestParamInfo<fs::path>& info) {
        return info.param.stem().string();
    }
);
