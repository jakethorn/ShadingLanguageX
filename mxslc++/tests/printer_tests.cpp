//
// Created by jaket on 28/09/2026.
//

#include "gtest/gtest.h"
#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>
#include "parse.h"
#include "scan.h"
#include "statements/Statement.h"
#include "utils/comp_utils.h"
#include "utils/data_utils.h"

namespace fs = std::filesystem;
using std::string;
using std::vector;

// The roundtrip test files are written in the same format that the decompiler prints, so parsing them and printing the
// statements with to_string should reproduce them, apart from whitespace and comments.
using printer_tests = testing::TestWithParam<fs::path>;

TEST_P(printer_tests, printed_source_matches_original)
{
    const fs::path& input_path = GetParam();

    const vector<mxslc::StmtPtr> statements = mxslc::parse(mxslc::scan_file(input_path));
    const string actual_output = mxslc::statements::join_statements(statements) + "\n";

    const string expected_output = read_file(input_path);
    // whitespace and comments do not have to be printed the same
    const bool passed = code_tokens(actual_output) == code_tokens(expected_output);

    EXPECT_TRUE(passed);
    if (not passed)
        print_debug_info(input_path, actual_output, expected_output);
}

vector<fs::path> get_printer_files()
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
    printer,
    printer_tests,
    testing::ValuesIn(get_printer_files()),
    [](const testing::TestParamInfo<fs::path>& info) {
        // e.g., basic001_decompiled for basic001.decompiled.mxsl
        string name = info.param.stem().string();
        std::replace(name.begin(), name.end(), '.', '_');
        return name;
    }
);
