//
// Created by jaket on 06/01/2026.
//

#include "gtest/gtest.h"
#include <filesystem>
#include <string>
#include <vector>
#include "decompile/decompile.h"
#include "utils/parse_cli_args.h"
#include "utils/comp_utils.h"
#include "utils/data_utils.h"
#include "utils/graph_utils.h"
#include "compile.h"
#include <MaterialXFormat/XmlIo.h>

namespace fs = std::filesystem;
using namespace std::string_literals;
using std::string;
using std::vector;

using decompiler_tests = testing::TestWithParam<fs::path>;

TEST_P(decompiler_tests, decompiler_output_matches_groundtruth)
{
    const fs::path& input_path = GetParam();
    fs::path expected_path = input_path;
    expected_path.replace_extension(".mxsl");

    const string actual_output = mxslc::decompile_to_string(input_path);

    if constexpr (overwrite_data_files())
        write_file(expected_path, actual_output);

    const string expected_output = normalise_line_endings(read_file(expected_path));
    const bool passed = actual_output == expected_output;

    EXPECT_TRUE(passed);
    if (not passed)
        print_debug_info(input_path, actual_output, expected_output);
}

TEST_P(decompiler_tests, decompiler_output_compiles_to_equivalent_graph)
{
    const fs::path& input_path = GetParam();

    const mx::DocumentPtr original = mx::createDocument();
    mx::readFromXmlFile(original, input_path.string());

    mxslc::CompileOptions opts;
    opts.reduce_graph = false;
    const string decompiled = mxslc::decompile_to_string(input_path);
    const mx::DocumentPtr recompiled = mxslc::compile_to_document(decompiled, opts);

    const vector<string> differences = GraphComparator::differences(original, recompiled);
    EXPECT_TRUE(differences.empty()) << "different elements: " << testing::PrintToString(differences) << "\n" << decompiled;
}

vector<fs::path> get_decompiler_files()
{
    const fs::path test_dir = get_test_data("decompiler");

    if (not fs::exists(test_dir))
        return {};

    vector<fs::path> files;
    for (const auto& p : fs::recursive_directory_iterator(test_dir))
        if (p.path().extension() == ".mtlx")
            files.push_back(p.path());

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
