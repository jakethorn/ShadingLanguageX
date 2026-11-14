//
// Created by jaket on 13/11/2026.
//

// Every MaterialX document in decompile_cases must compile to the same graph after it is decompiled. Each document is a
// small case that the decompiler must handle, which its comment describes.

#include "gtest/gtest.h"
#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>
#include <MaterialXFormat/XmlIo.h>
#include "compile.h"
#include "CompileOptions.h"
#include "decompile/decompile.h"
#include "utils/data_utils.h"
#include "utils/graph_utils.h"

namespace fs = std::filesystem;
using std::string;
using std::vector;

using decompile_cases_tests = testing::TestWithParam<fs::path>;

TEST_P(decompile_cases_tests, decompiled_code_compiles_to_the_same_graph)
{
    const mx::DocumentPtr original = mx::createDocument();
    mx::readFromXmlFile(original, GetParam().string());

    const string decompiled = mxslc::decompile_to_string(original);
    mxslc::CompileOptions opts;
    opts.reduce_graph = false;
    const mx::DocumentPtr recompiled = mxslc::compile_to_document(decompiled, opts);

    EXPECT_EQ(GraphComparator::find_differences(original, recompiled), vector<string>{}) << decompiled;
}

namespace
{
    vector<fs::path> get_case_files()
    {
        vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(get_test_data("decompile_cases")))
            if (entry.path().extension() == ".mtlx")
                files.push_back(entry.path());
        std::sort(files.begin(), files.end());
        return files;
    }
}

INSTANTIATE_TEST_SUITE_P(
    decompiler,
    decompile_cases_tests,
    testing::ValuesIn(get_case_files()),
    [](const testing::TestParamInfo<fs::path>& info) {
        return info.param.stem().string();
    }
);
