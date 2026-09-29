//
// Created by jaket on 29/09/2026.
//

// Every groundtruth file must compile to the same graph after it is decompiled, except the known exceptions, which are
// listed by name in decompile_exceptions.txt. Known exceptions are skipped while they still fail, and fail once they pass,
// so that the list is kept up to date.

#include "gtest/gtest.h"
#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_set>
#include <vector>
#include "compile.h"
#include "CompileOptions.h"
#include "decompile/decompile.h"
#include "utils/parse_cli_args.h"
#include "utils/data_utils.h"
#include "utils/graph_utils.h"

namespace fs = std::filesystem;
using std::string;
using std::vector;

using groundtruth_decompile_tests = testing::TestWithParam<fs::path>;

namespace
{
    const std::unordered_set<string>& known_exceptions()
    {
        static const std::unordered_set<string> exceptions = [] {
            std::unordered_set<string> result;
            std::ifstream file{get_test_data("decompile_exceptions.txt")};
            string line;
            while (std::getline(file, line))
            {
                line = line.substr(0, line.find('#'));
                line.erase(line.find_last_not_of(" \t\r") + 1);
                if (not line.empty())
                    result.insert(line);
            }
            return result;
        }();
        return exceptions;
    }

    mxslc::CompileOptions compile_options(const fs::path& input_path)
    {
        mxslc::CompileOptions opts;
        opts.reduce_graph = false;

        fs::path response_path = input_path;
        response_path.replace_extension(".rsp");
        if (fs::is_regular_file(response_path))
        {
            const mxslc::CommandLineArgs args = mxslc::parse_cli_args(response_path);
            if (args.is_valid)
                opts = args.options;
        }

        return opts;
    }
}

TEST_P(groundtruth_decompile_tests, decompiled_code_compiles_to_the_same_graph)
{
    const fs::path& input_path = GetParam();
    const string name = input_path.stem().string();

    mx::DocumentPtr original;
    try
    {
        original = mxslc::compile_to_document(input_path, compile_options(input_path));
    }
    catch (const std::exception&)
    {
        GTEST_SKIP() << "the groundtruth file does not compile";
    }

    // the decompiled code is compiled without the options of the original code, e.g., its entry function
    string decompiled;
    string error;
    vector<string> differences;
    try
    {
        decompiled = mxslc::decompile_to_string(original);
        mxslc::CompileOptions opts;
        opts.reduce_graph = false;
        const mx::DocumentPtr recompiled = mxslc::compile_to_document(decompiled, opts);
        differences = GraphComparator::differences(original, recompiled);
    }
    catch (const std::exception& e)
    {
        error = e.what();
    }
    const bool passed = error.empty() and differences.empty();

    if (known_exceptions().count(name) > 0)
    {
        if (passed)
            FAIL() << name << " compiles to the same graph now, remove it from decompile_exceptions.txt";
        GTEST_SKIP() << "known exception";
    }

    EXPECT_TRUE(passed) << error << "different elements: " << testing::PrintToString(differences) << "\n" << decompiled;
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
        std::sort(files.begin(), files.end());

        return files;
    }
}

INSTANTIATE_TEST_SUITE_P(
    decompiler,
    groundtruth_decompile_tests,
    testing::ValuesIn(get_groundtruth_files()),
    [](const testing::TestParamInfo<fs::path>& info) {
        return info.param.stem().string();
    }
);
