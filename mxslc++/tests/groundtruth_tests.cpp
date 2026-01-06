//
// Created by jaket on 06/01/2026.
//

#include "gtest/gtest.h"
#include <filesystem>
#include <string>
#include <vector>
#include "compile.h"
#include "utils/data_utils.h"

namespace fs = std::filesystem;
using namespace std::string_literals;
using std::string;
using std::vector;

using GroundTruthTest = testing::TestWithParam<fs::path>;

TEST_P(GroundTruthTest, CompilesToMatchGroundTruth)
{
    fs::path path = GetParam();
    const fs::path input_path = path;
    const fs::path actual_path = path.replace_extension(".tmp");
    const fs::path expected_path = path.replace_extension(".mtlx");

    ASSERT_NO_THROW(mxslc::compile(input_path, actual_path))
        << "Failed to compile: " << input_path;

    const string actual_output = read_file(actual_path);
    const string expected_output = read_file(expected_path);

    EXPECT_EQ(actual_output, expected_output)
        << "Mismatch in file: " << input_path.filename();

    fs::remove(actual_path);
}

vector<fs::path> get_test_files()
{
    vector<fs::path> files;
    const fs::path test_dir = get_test_data("groundtruth"s);

    for (const auto& p : fs::recursive_directory_iterator(test_dir))
        if (p.path().extension() == ".mxsl")
            files.push_back(p.path());

    return files;
}

INSTANTIATE_TEST_SUITE_P(
    AllFiles,
    GroundTruthTest,
    testing::ValuesIn(get_test_files()),
    [](const testing::TestParamInfo<fs::path>& info) {
        string name = info.param.stem().string();
        std::replace(name.begin(), name.end(), '-', '_');
        return name;
    }
);