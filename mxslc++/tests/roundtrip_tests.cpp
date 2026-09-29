//
// Created by jaket on 06/01/2026.
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
#include "utils/graph_utils.h"

namespace fs = std::filesystem;
using namespace std::string_literals;
using std::string;
using std::vector;

using roundtrip_tests = testing::TestWithParam<fs::path>;

namespace
{
    // files in the hints folder can only be decompiled to their original code with decompile hints
    bool requires_decompile_hints(const fs::path& path)
    {
        return path.parent_path().filename() == "hints";
    }

    mxslc::CompileOptions roundtrip_options(const bool decompile_hints)
    {
        mxslc::CompileOptions opts;
        opts.reduce_graph = false;
        opts.decompile_hints = decompile_hints;
        return opts;
    }
}

TEST_P(roundtrip_tests, roundtrip_output_matches_groundtruth)
{
    const fs::path& input_path = GetParam();
    const string expected_output = read_file(input_path);

    // files that do not require hints must be decompiled the same with or without them
    const vector<bool> hint_options = requires_decompile_hints(input_path) ? vector{true} : vector{false, true};
    for (const bool decompile_hints : hint_options)
    {
        const string mtlx = mxslc::compile_to_string(input_path, roundtrip_options(decompile_hints));
        const string actual_output = mxslc::decompile_to_string(mtlx);

        // whitespace and comments do not have to be roundtripped
        const bool passed = code_tokens(actual_output) == code_tokens(expected_output);
        EXPECT_TRUE(passed) << "decompile_hints = " << std::boolalpha << decompile_hints;
        if (not passed)
            print_debug_info(input_path, actual_output, expected_output);
    }
}

TEST_P(roundtrip_tests, roundtrip_output_compiles_to_equivalent_graph)
{
    // even without hints, the decompiled code must compile to the same graph as the original code
    const fs::path& input_path = GetParam();
    const mxslc::CompileOptions opts = roundtrip_options(false);

    const mx::DocumentPtr original = mxslc::compile_to_document(input_path, opts);
    const string decompiled = mxslc::decompile_to_string(original);
    const mx::DocumentPtr recompiled = mxslc::compile_to_document(decompiled, opts);

    const vector<string> differences = GraphComparator::differences(original, recompiled);
    EXPECT_TRUE(differences.empty()) << "different elements: " << testing::PrintToString(differences) << "\n" << decompiled;
}

vector<fs::path> get_roundtrip_files()
{
    const fs::path test_dir = get_test_data("roundtrip");

    if (not fs::exists(test_dir))
        return {};

    vector<fs::path> files;
    for (const auto& p : fs::recursive_directory_iterator(test_dir))
        if (p.path().extension() == ".mxsl")
            files.push_back(p.path());

    return files;
}

INSTANTIATE_TEST_SUITE_P(
    roundtrip,
    roundtrip_tests,
    testing::ValuesIn(get_roundtrip_files()),
    [](const testing::TestParamInfo<fs::path>& info) {
        return info.param.stem().string();
    }
);
