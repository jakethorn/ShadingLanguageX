//
// Created by jaket on 05/10/2026.
//

// A function or node that is decompiled with its dependencies must compile on its own, so the decompiled code must
// include the functions that it calls, the nodes that it uses and the nonlocal variables that it reads, before it.

#include "gtest/gtest.h"
#include <filesystem>
#include <string>
#include <vector>
#include "compile.h"
#include "CompileOptions.h"
#include "decompile/Decompiler.h"
#include "utils/data_utils.h"
#include "utils/graph_utils.h"

namespace fs = std::filesystem;
using std::string;
using std::vector;

using decompile_dependencies_tests = testing::TestWithParam<fs::path>;

namespace
{
    mxslc::CompileOptions get_compile_options()
    {
        mxslc::CompileOptions opts;
        opts.reduce_graph = false;
        return opts;
    }

    // the error of the code, or nothing if it compiles
    string get_compile_error(const string& code)
    {
        try
        {
            mxslc::compile_to_document(code, get_compile_options());
            return "";
        }
        catch (const std::exception& e)
        {
            return e.what();
        }
    }

    // the elements come before the elements that depend on them, so that the decompiler cannot rely on their order
    mx::DocumentPtr compile_reordered(const fs::path& path)
    {
        return reverse_element_order(mxslc::compile_to_document(read_file(path), get_compile_options()));
    }
}

TEST_P(decompile_dependencies_tests, functions_are_decompiled_with_their_dependencies)
{
    const mx::DocumentPtr document = compile_reordered(GetParam());
    mxslc::decompile::Decompiler decompiler{document};

    for (const mx::NodeDefPtr& node_def : document->getNodeDefs())
    {
        const string code = decompiler.decompile_node_def(node_def, /*with_dependencies*/true);
        const string error = get_compile_error(code);
        EXPECT_TRUE(error.empty()) << node_def->getName() << ": " << error << "\n" << code;
    }

    for (const mx::NodeGraphPtr& node_graph : document->getNodeGraphs())
    {
        if (node_graph->getNodeDef())
            continue;

        const string code = decompiler.decompile_node_graph(node_graph, /*with_dependencies*/true);
        const string error = get_compile_error(code);
        EXPECT_TRUE(error.empty()) << node_graph->getName() << ": " << error << "\n" << code;
    }
}

TEST_P(decompile_dependencies_tests, nodes_are_decompiled_with_their_dependencies)
{
    const mx::DocumentPtr document = compile_reordered(GetParam());
    mxslc::decompile::Decompiler decompiler{document};

    for (const mx::NodePtr& node : document->getNodes())
    {
        const string code = decompiler.decompile_node(node, /*with_dependencies*/true);
        const string error = get_compile_error(code);
        EXPECT_TRUE(error.empty()) << node->getName() << ": " << error << "\n" << code;
    }
}

namespace
{
    vector<fs::path> get_roundtrip_files()
    {
        const fs::path test_dir = get_test_data("roundtrip");

        if (not fs::exists(test_dir))
            return {};

        vector<fs::path> files;
        for (const auto& p : fs::directory_iterator(test_dir))
        {
            const string name = p.path().filename().string();
            if (p.path().extension() == ".mxsl" and name.find(".decompiled.") == string::npos)
                files.push_back(p.path());
        }
        std::sort(files.begin(), files.end());

        return files;
    }
}

INSTANTIATE_TEST_SUITE_P(
    decompiler,
    decompile_dependencies_tests,
    testing::ValuesIn(get_roundtrip_files()),
    [](const testing::TestParamInfo<fs::path>& info) {
        return info.param.stem().string();
    }
);
