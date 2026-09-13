//
// Created for MXSL decompiler round-trip testing.
//

#include "gtest/gtest.h"
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

#include "compile.h"
#include "CompileOptions.h"
#include "decompile/decompile.h"
#include "utils/parse_cli_args.h"
#include "utils/comp_utils.h"
#include "utils/data_utils.h"

namespace fs = std::filesystem;
using namespace std::string_literals;
using std::string;
using std::vector;

using decompiler_tests = testing::TestWithParam<fs::path>;

TEST_P(decompiler_tests, decompiler_output_matches_groundtruth)
{
    const fs::path& input_path = GetParam();

    mxslc::CompileOptions opts;
    opts.reduce_graph = false;
    opts.emit_source_hints = true;

    const fs::path response_path = fs::path(input_path).replace_extension(".rsp");
    if (fs::is_regular_file(response_path))
    {
        const mxslc::CommandLineArgs args = mxslc::parse_cli_args(response_path);
        ASSERT_TRUE(args.is_valid) << "Errors in response file: " << response_path.string();
        opts = args.options;
        opts.emit_source_hints = true;
    }

    const string original_mxsl = read_file(input_path);

    // 1. Compile MXSL to MaterialX (MTLX 1)
    string mtlx1;
    try
    {
        mtlx1 = mxslc::compile_to_string(input_path, opts);
    }
    catch (const std::exception& e)
    {
        FAIL() << "Initial compilation of MXSL failed: " << e.what() << "\nFile: " << input_path.string();
    }
    ASSERT_FALSE(mtlx1.empty()) << "Compiled MTLX is empty for file: " << input_path.string();

    // 2. Decompile MTLX 1 to MXSL (Decompiled MXSL)
    string actual_decompiled;
    try
    {
        actual_decompiled = mxslc::decompile::decompile_to_string(mtlx1);
    }
    catch (const std::exception& e)
    {
        FAIL() << "Decompilation of MTLX failed: " << e.what() << "\nMTLX:\n" << mtlx1;
    }
    ASSERT_FALSE(actual_decompiled.empty()) << "Decompiled MXSL is empty for file: " << input_path.string();

    // 3. Expected decompiled MXSL comparison
    fs::path expected_decompiled_path = input_path;
    expected_decompiled_path.replace_filename(input_path.stem().string() + "_decompiled.mxsl");

    if constexpr (overwrite_data_files())
        write_file(expected_decompiled_path, actual_decompiled);

    const string expected_output = fs::is_regular_file(expected_decompiled_path)
        ? read_file(expected_decompiled_path)
        : original_mxsl;

    const bool mxsl_passed = (trim(actual_decompiled) == trim(expected_output));
    EXPECT_TRUE(mxsl_passed);
    if (!mxsl_passed)
        print_debug_info(input_path, actual_decompiled, expected_output);

    // 4. Recompile Decompiled MXSL to MaterialX (MTLX 2)
    string mtlx2;
    try
    {
        mtlx2 = mxslc::compile_to_string(actual_decompiled, opts);
    }
    catch (const std::exception& e)
    {
        FAIL() << "Recompilation of decompiled MXSL failed: " << e.what()
               << "\nDecompiled MXSL:\n" << actual_decompiled
               << "\nOriginal MTLX:\n" << mtlx1;
    }

    // 5. Verify MaterialX semantic equivalence (MTLX 1 == MTLX 2)
    const fs::path no_mtlx_identity_path = fs::path(input_path).replace_extension(".no_mtlx_identity");
    if (!fs::is_regular_file(no_mtlx_identity_path))
    {
        const bool mtlx_passed = (mtlx1 == mtlx2);
        EXPECT_TRUE(mtlx_passed) << "Recompiled MTLX does not match original MTLX for: " << input_path.string();
        if (!mtlx_passed)
        {
            std::cout << "\n================ [MTLX RECOMPILATION MISMATCH: " << input_path.stem().string() << "] ================\n";
            std::cout << column_compare("Original MTLX", mtlx1, "Recompiled MTLX", mtlx2, true);
        }
    }

    // 6. Verify Decompiler Idempotency / Fixpoint (Decompile(MTLX 2) == actual_decompiled)
    string actual_decompiled2;
    try
    {
        actual_decompiled2 = mxslc::decompile::decompile_to_string(mtlx2);
    }
    catch (const std::exception& e)
    {
        FAIL() << "Second decompilation failed: " << e.what();
    }

    EXPECT_EQ(actual_decompiled, actual_decompiled2)
        << "Decompiler is not idempotent (fixpoint test failed) for: " << input_path.string();
}

vector<fs::path> get_decompiler_files()
{
    const fs::path test_dir = get_test_data("decompiler");

    if (!fs::exists(test_dir))
        return {};

    vector<fs::path> files;
    for (const auto& p : fs::recursive_directory_iterator(test_dir))
    {
        if (p.is_regular_file() && p.path().extension() == ".mxsl")
        {
            const string filename = p.path().filename().string();
            if (filename.find("_decompiled.") == string::npos &&
                filename.find(".decompiled.") == string::npos)
            {
                files.push_back(p.path());
            }
        }
    }
    std::sort(files.begin(), files.end());
    return files;
}

INSTANTIATE_TEST_SUITE_P(
    decompiler,
    decompiler_tests,
    testing::ValuesIn(get_decompiler_files()),
    [](const testing::TestParamInfo<fs::path>& info) {
        return info.param.stem().string();
    }
);
