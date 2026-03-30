//
// Created by jaket on 06/01/2026.
//

#include "gtest/gtest.h"
#include <filesystem>
#include <string>
#include <vector>
#include "compile.h"
#include "utils/comp_utils.h"
#include "utils/data_utils.h"

namespace fs = std::filesystem;
using namespace std::string_literals;
using std::string;
using std::vector;

void print_debug_info(const fs::path& input_path, const string& actual_output, const string& expected_output)
{
    const string border = string(input_path.filename().string().size() + 4, '-');
    std::cout
    << "\n\n\n"
    << border
    << "\n"
    << "| " << input_path.filename().string() << " |"
    << "\n"
    << border
    << "\n\n"
    << column_compare("Actual Output"s, actual_output, "Expected Output"s, expected_output, true)
    << "\n\n"
    << column_compare("Actual Output"s, actual_output, "Input"s, read_file(input_path), false)
    << "\n\n\n";
}

using groundtruth_tests = testing::TestWithParam<fs::path>;

TEST_P(groundtruth_tests, compiler_output_matches_groundtruth)
{
    fs::path path = GetParam();
    const fs::path input_path = path;
    const fs::path actual_path = path.replace_extension(".tmp");
    const fs::path expected_path = path.replace_extension(".mtlx");

    ASSERT_NO_THROW(mxslc::compile_to_file(input_path, actual_path))
        << "Failed to compile: " << input_path;

    const string actual_output = read_file(actual_path);
    if (overwrite_data_files())
    {
        write_file(expected_path, actual_output);
    }
    const string expected_output = read_file(expected_path);

    print_debug_info(input_path, actual_output, expected_output);
    EXPECT_EQ(actual_output, expected_output);

    fs::remove(actual_path);
}

vector<fs::path> get_mxsl_files()
{
    vector<fs::path> files;
    const fs::path test_dir = get_test_data("groundtruth"s);

    for (const auto& p : fs::recursive_directory_iterator(test_dir))
        if (p.path().extension() == ".mxsl")
            files.push_back(p.path());

    return files;
}

INSTANTIATE_TEST_SUITE_P(
    compiler,
    groundtruth_tests,
    testing::ValuesIn(get_mxsl_files()),
    [](const testing::TestParamInfo<fs::path>& info) {
        return info.param.stem().string();
    }
);
