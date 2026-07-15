//
// Created by jaket on 28/05/2026.
//

#include "gtest/gtest.h"
#include <filesystem>
#include <string>
#include <vector>
#include "compile.h"
#include "CompileOptions.h"
#include "utils/comp_utils.h"
#include "utils/data_utils.h"

namespace fs = std::filesystem;
using namespace std::string_literals;
using std::string;
using std::vector;

using reduce_graph_tests = testing::TestWithParam<fs::path>;

void run_test(const fs::path& input_path, const fs::path& expected_path, const bool reduce_graph, const string& fail_message)
{
    const mxslc::CompileOptions opts{.reduce_graph = reduce_graph};
    const string actual_output = mxslc::compile_to_string(input_path, opts);

    if constexpr (overwrite_data_files())
        write_file(expected_path, actual_output);

    const string expected_output = read_file(expected_path);
    const bool passed = actual_output == expected_output;

    EXPECT_TRUE(passed) << fail_message;

    if (not passed)
        print_debug_info(input_path, actual_output, expected_output);
}

TEST_P(reduce_graph_tests, compiler_reduces_graph)
{
    const fs::path& input_path = GetParam();

    fs::path expected_std_path = input_path;
    expected_std_path.replace_extension(".mtlx");

    if (fs::is_regular_file(expected_std_path))
        run_test(input_path, expected_std_path, false, "Standard compile failed."s);
    else
        std::cout << "No standard file found.";

    fs::path expected_reduced_path = input_path;
    expected_reduced_path.replace_filename(expected_reduced_path.stem().string() + "_reduced"s);
    expected_reduced_path.replace_extension(".mtlx");

    run_test(input_path, expected_reduced_path, true, "Reduced compile failed."s);
}

vector<fs::path> get_reduce_graph_files()
{
    const fs::path test_dir = get_test_data("reduce_graph");

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
    reduce_graph_tests,
    testing::ValuesIn(get_reduce_graph_files()),
    [](const testing::TestParamInfo<fs::path>& info) {
        return info.param.stem().string();
    }
);
