//
// Created by jaket on 09/04/2026.
//

#include "gtest/gtest.h"
#include <filesystem>
#include <string>
#include <vector>
#include "compile.h"
#include "CompileError.h"
#include "utils/data_utils.h"

namespace fs = std::filesystem;
using namespace std::string_literals;
using std::string;
using std::vector;

using failure_tests = testing::TestWithParam<fs::path>;

TEST_P(failure_tests, compiler_throws_compile_error)
{
    fs::path path = GetParam();
    const fs::path input_path = path;
    const fs::path actual_path = path.replace_extension(".tmp");

    EXPECT_THROW(
        {
            try
            {
                mxslc::compile_to_file(input_path, actual_path);
            }
            catch(const mxslc::CompileError& e)
            {
                std::cout << "Caught expected error: " << e.what() << std::endl;
                throw;
            }
        },
        mxslc::CompileError
    ) << "\nFailed to throw CompileError: " << input_path << "\n\n" << read_file(input_path);

    if (fs::exists(actual_path))
        fs::remove(actual_path);
}

vector<fs::path> get_failure_files()
{
    vector<fs::path> files;
    const fs::path test_dir = get_test_data("failure"s);

    if (fs::exists(test_dir)) {
        for (const auto& p : fs::recursive_directory_iterator(test_dir))
            if (p.path().extension() == ".mxsl")
                files.push_back(p.path());
    }
    return files;
}

INSTANTIATE_TEST_SUITE_P(
    compiler,
    failure_tests,
    testing::ValuesIn(get_failure_files()),
    [](const testing::TestParamInfo<fs::path>& info) {
        return info.param.stem().string();
    }
);