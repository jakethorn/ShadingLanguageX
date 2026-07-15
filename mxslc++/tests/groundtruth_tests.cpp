//
// Created by jaket on 06/01/2026.
//

#include "gtest/gtest.h"
#include <filesystem>
#include <string>
#include <vector>
#include "compile.h"
#include "utils/parse_cli_args.h"
#include "utils/comp_utils.h"
#include "utils/data_utils.h"

namespace fs = std::filesystem;
using namespace std::string_literals;
using std::string;
using std::vector;

using groundtruth_tests = testing::TestWithParam<fs::path>;

TEST_P(groundtruth_tests, compiler_output_matches_groundtruth)
{
    const fs::path& input_path = GetParam();
    fs::path expected_path = input_path;
    expected_path.replace_extension(".mtlx");

    mxslc::CompileOptions opts{.reduce_graph = false};
    fs::path response_path = input_path;
    response_path.replace_extension(".rsp");
    if (fs::is_regular_file(response_path))
        opts = mxslc::parse_cli_args(response_path).options;

    const string actual_output = mxslc::compile_to_string(input_path, opts);

    if constexpr (overwrite_data_files())
        write_file(expected_path, actual_output);

    const string expected_output = read_file(expected_path);
    const bool passed = actual_output == expected_output;

    EXPECT_TRUE(passed);
    if (not passed)
        print_debug_info(input_path, actual_output, expected_output);
}

vector<fs::path> get_groundtruth_files()
{
    const fs::path test_dir = get_test_data("groundtruth");

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
    groundtruth_tests,
    testing::ValuesIn(get_groundtruth_files()),
    [](const testing::TestParamInfo<fs::path>& info) {
        return info.param.stem().string();
    }
);
