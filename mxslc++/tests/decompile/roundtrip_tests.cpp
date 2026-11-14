//
// Created by jaket on 06/01/2026.
//

// Each roundtrip test is a pair of files: the original code, <name>.mxsl, and the code it is decompiled to,
// <name>.decompiled.mxsl, which is created by the decompiler (see overwrite_data_files) and then reviewed. The decompiled
// code does not have to be the same as the original code, e.g., inline functions and loops are unrolled, but it must
// compile to the same graph.

#include "gtest/gtest.h"
#include <filesystem>
#include <string>
#include <vector>
#include "compile.h"
#include "CompileOptions.h"
#include "decompile/decompile.h"
#include "utils/comp_utils.h"
#include "utils/data_utils.h"
#include "utils/graph_utils.h"

namespace fs = std::filesystem;
using std::string;
using std::vector;

using roundtrip_tests = testing::TestWithParam<fs::path>;

namespace
{
    const string DECOMPILED_EXTENSION = ".decompiled.mxsl";

    fs::path get_decompiled_path(const fs::path& original_path)
    {
        return fs::path{original_path}.replace_extension(DECOMPILED_EXTENSION);
    }

    bool is_decompiled_file(const fs::path& path)
    {
        const string name = path.filename().string();
        return name.size() >= DECOMPILED_EXTENSION.size() and name.compare(name.size() - DECOMPILED_EXTENSION.size(), DECOMPILED_EXTENSION.size(), DECOMPILED_EXTENSION) == 0;
    }

    mxslc::CompileOptions get_roundtrip_options()
    {
        mxslc::CompileOptions opts;
        opts.reduce_graph = false;
        return opts;
    }

    string decompile(const string& code)
    {
        return mxslc::decompile_to_string(mxslc::compile_to_document(code, get_roundtrip_options()));
    }
}

TEST_P(roundtrip_tests, decompiled_code_matches_expected)
{
    const fs::path& original_path = GetParam();
    const string actual_output = decompile(read_file(original_path));

    if constexpr (overwrite_data_files())
        write_file(get_decompiled_path(original_path), actual_output);

    // whitespace and comments do not have to be the same
    const string expected_output = read_file(get_decompiled_path(original_path));
    const bool passed = get_code_tokens(actual_output) == get_code_tokens(expected_output);
    EXPECT_TRUE(passed);
    if (not passed)
        print_debug_info(original_path, actual_output, expected_output);
}

TEST_P(roundtrip_tests, decompiled_code_compiles_to_the_same_graph)
{
    const fs::path& original_path = GetParam();

    const mx::DocumentPtr original = mxslc::compile_to_document(read_file(original_path), get_roundtrip_options());
    const string decompiled = read_file(get_decompiled_path(original_path));
    const mx::DocumentPtr recompiled = mxslc::compile_to_document(decompiled, get_roundtrip_options());

    const vector<string> differences = GraphComparator::find_differences(original, recompiled);
    EXPECT_TRUE(differences.empty()) << "different elements: " << testing::PrintToString(differences) << "\n" << decompiled;
}

TEST_P(roundtrip_tests, decompiling_decompiled_code_gives_the_same_code)
{
    // the decompiled code is already in the form that the decompiler creates
    const fs::path& original_path = GetParam();
    const string decompiled = read_file(get_decompiled_path(original_path));
    const string redecompiled = decompile(decompiled);

    const bool passed = get_code_tokens(redecompiled) == get_code_tokens(decompiled);
    EXPECT_TRUE(passed);
    if (not passed)
        print_debug_info(get_decompiled_path(original_path), redecompiled, decompiled);
}

vector<fs::path> get_roundtrip_files()
{
    const fs::path test_dir = get_test_data("roundtrip");

    if (not fs::exists(test_dir))
        return {};

    vector<fs::path> files;
    for (const auto& p : fs::directory_iterator(test_dir))
        if (p.path().extension() == ".mxsl" and not is_decompiled_file(p.path()))
            files.push_back(p.path());
    std::sort(files.begin(), files.end());

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
