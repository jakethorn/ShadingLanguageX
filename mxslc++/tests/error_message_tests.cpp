//
// Created by jaket on 10/04/2026.
//

#include "gtest/gtest.h"
#include <filesystem>
#include <string>
#include <vector>
#include "compile.h"
#include "CompileError.h"
#include "utils/comp_utils.h"
#include "utils/data_utils.h"

namespace fs = std::filesystem;
using namespace std::string_literals;
using std::string;
using std::vector;

using error_message_tests = testing::TestWithParam<fs::path>;

TEST_P(error_message_tests, error_message_matches_groundtruth)
{
    fs::path path = GetParam();
    const fs::path input_path = path;
    const fs::path actual_path = path.replace_extension(".tmp");
    const fs::path expected_path = path.replace_extension(".txt");

    string actual_output;
    EXPECT_THROW(
        {
            try
            {
                mxslc::compile_to_file(input_path, actual_path);
            }
            catch(const mxslc::CompileError& e)
            {
                actual_output = e.what();
                std::cout << "Caught expected error: " << e.what() << std::endl;
                throw;
            }
        },
        mxslc::CompileError
    ) << "Failed to throw CompileError: " << input_path;

    if (overwrite_data_files())
    {
        write_file(expected_path, actual_output);
    }
    const string expected_output = read_file(expected_path);

    print_debug_info(input_path, actual_output, expected_output);
    EXPECT_EQ(actual_output, expected_output);

    if (fs::exists(actual_path))
        fs::remove(actual_path);
}

vector<fs::path> get_error_message_files()
{
    vector<fs::path> files;
    const fs::path test_dir = get_test_data("error_message"s);

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
