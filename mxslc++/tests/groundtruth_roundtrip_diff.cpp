//
// Created by jaket on 28/09/2026.
//

#include "gtest/gtest.h"
#include <filesystem>
#include <string>
#include <vector>
#include "compile.h"
#include "decompile/decompile.h"
#include "utils/parse_cli_args.h"
#include "utils/comp_utils.h"
#include "utils/data_utils.h"

namespace fs = std::filesystem;
using namespace std::string_literals;
using std::string;
using std::vector;

using groundtruth_roundtrip_diffs = testing::TestWithParam<fs::path>;

TEST_P(groundtruth_roundtrip_diffs, see_groundtruth_roundtrip_diffs)
{
    // This isn't really a test, but a convenient way to see the roundtrip diffs for all groundtruth tests,
    // set the #if check to false and run to see the diffs.
#if true
    GTEST_SKIP();
#endif

    const fs::path& input_path = GetParam();

    mxslc::CompileOptions opts;
    opts.reduce_graph = false;
    opts.decompile_hints = true;

    fs::path response_path = input_path;
    response_path.replace_extension(".rsp");
    if (fs::is_regular_file(response_path))
    {
        const mxslc::CommandLineArgs args = mxslc::parse_cli_args(response_path);
        ASSERT_TRUE(args.is_valid) << "Errors in response file: " << response_path.string();
        opts = args.options;
    }

    const string mtlx = mxslc::compile_to_string(input_path, opts);
    const string actual_output = mxslc::decompile_to_string(mtlx);

    const string expected_output = read_file(input_path);
    print_debug_info(input_path, actual_output, expected_output);
}

namespace
{
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
}

INSTANTIATE_TEST_SUITE_P(
    decompiler,
    groundtruth_roundtrip_diffs,
    testing::ValuesIn(get_groundtruth_files()),
    [](const testing::TestParamInfo<fs::path>& info) {
        return info.param.stem().string();
    }
);
