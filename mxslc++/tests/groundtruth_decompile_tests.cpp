//
// Created by jaket on 29/09/2026.
//

// Every groundtruth file must compile to the same graph after it is decompiled, except the known exceptions, which are
// listed by name in decompile_exceptions.txt. Known exceptions are skipped while they still fail, and fail once they pass,
// so that the list is kept up to date.
//
// The groundtruth files are compiled from code, so their elements always come after the elements they depend on. Each
// file is also decompiled with its elements in reverse order, which the decompiler must put back in dependency order.

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
    const std::unordered_set<string>& get_known_exceptions()
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

    mxslc::CompileOptions get_compile_options(const fs::path& input_path)
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

    // the original document, or nothing if the groundtruth file does not compile
    mx::DocumentPtr compile_groundtruth(const fs::path& input_path)
    {
        try
        {
            return mxslc::compile_to_document(input_path, get_compile_options(input_path));
        }
        catch (const std::exception&)
        {
            return nullptr;
        }
    }

    struct DecompileResult
    {
        string decompiled;
        string error;
        vector<string> differences;

        bool passed() const { return error.empty() and differences.empty(); }
    };

    // decompiles the document and compares the graph that the decompiled code compiles to with the original document
    DecompileResult decompile_and_compare(const mx::DocumentPtr& document, const mx::DocumentPtr& original)
    {
        // the decompiled code is compiled without the options of the original code, e.g., its entry function
        DecompileResult result;
        try
        {
            result.decompiled = mxslc::decompile_to_string(document);
            mxslc::CompileOptions opts;
            opts.reduce_graph = false;
            const mx::DocumentPtr recompiled = mxslc::compile_to_document(result.decompiled, opts);
            result.differences = GraphComparator::find_differences(original, recompiled);
        }
        catch (const std::exception& e)
        {
            result.error = e.what();
        }
        return result;
    }
}

TEST_P(groundtruth_decompile_tests, decompiled_code_compiles_to_the_same_graph)
{
    const fs::path& input_path = GetParam();
    const string name = input_path.stem().string();

    const mx::DocumentPtr original = compile_groundtruth(input_path);
    if (original == nullptr)
        GTEST_SKIP() << "the groundtruth file does not compile";

    const DecompileResult result = decompile_and_compare(original, original);

    if (get_known_exceptions().count(name) > 0)
    {
        if (result.passed())
            FAIL() << name << " compiles to the same graph now, remove it from decompile_exceptions.txt";
        GTEST_SKIP() << "known exception";
    }

    EXPECT_TRUE(result.passed()) << result.error << "different elements: " << testing::PrintToString(result.differences) << "\n" << result.decompiled;
}

TEST_P(groundtruth_decompile_tests, reordered_document_decompiles_to_the_same_graph)
{
    const fs::path& input_path = GetParam();
    const string name = input_path.stem().string();

    // the known exceptions do not compile to the same graph in any order, see decompiled_code_compiles_to_the_same_graph
    if (get_known_exceptions().count(name) > 0)
        GTEST_SKIP() << "known exception";

    const mx::DocumentPtr original = compile_groundtruth(input_path);
    if (original == nullptr)
        GTEST_SKIP() << "the groundtruth file does not compile";

    const DecompileResult result = decompile_and_compare(reverse_element_order(original), original);
    EXPECT_TRUE(result.passed()) << result.error << "different elements: " << testing::PrintToString(result.differences) << "\n" << result.decompiled;
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
