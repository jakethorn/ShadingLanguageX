//
// Created by jaket on 05/10/2026.
//

// Each file in data/decompile_dependencies is a document whose elements come before the elements that they depend on,
// e.g., a node before the node def of the function that it calls, which never happens in documents that are compiled from
// code. The decompiled code must still define everything before it is used, so the document must decompile to code that
// compiles to the same graph, and each of its functions and nodes must decompile with its dependencies to code that
// compiles on its own.

#include "gtest/gtest.h"
#include <filesystem>
#include <string>
#include <vector>
#include <MaterialXFormat/XmlIo.h>
#include "compile.h"
#include "CompileOptions.h"
#include "decompile/decompile.h"
#include "decompile/Decompiler.h"
#include "utils/data_utils.h"
#include "utils/graph_utils.h"

namespace fs = std::filesystem;
using std::string;
using std::vector;

using decompile_dependencies_tests = testing::TestWithParam<fs::path>;

namespace
{
    mx::DocumentPtr read_document(const fs::path& path)
    {
        const mx::DocumentPtr document = mx::createDocument();
        mx::readFromXmlFile(document, path.string());
        return document;
    }

    mx::DocumentPtr compile(const string& code)
    {
        mxslc::CompileOptions opts;
        opts.reduce_graph = false;
        return mxslc::compile_to_document(code, opts);
    }

    // the error of the code, or nothing if it compiles
    string get_compile_error(const string& code)
    {
        try
        {
            compile(code);
            return "";
        }
        catch (const std::exception& e)
        {
            return e.what();
        }
    }
}

TEST_P(decompile_dependencies_tests, decompiled_code_compiles_to_the_same_graph)
{
    const mx::DocumentPtr document = read_document(GetParam());
    const string decompiled = mxslc::decompile_to_string(document);

    mx::DocumentPtr recompiled;
    try
    {
        recompiled = compile(decompiled);
    }
    catch (const std::exception& e)
    {
        FAIL() << e.what() << "\n" << decompiled;
    }

    const vector<string> differences = GraphComparator::find_differences(document, recompiled);
    EXPECT_TRUE(differences.empty()) << "different elements: " << testing::PrintToString(differences) << "\n" << decompiled;
}

TEST_P(decompile_dependencies_tests, functions_and_nodes_are_decompiled_with_their_dependencies)
{
    const mx::DocumentPtr document = read_document(GetParam());
    mxslc::decompile::Decompiler decompiler{document};

    const auto expect_compiles = [](const string& name, const string& code) {
        const string error = get_compile_error(code);
        EXPECT_TRUE(error.empty()) << name << ": " << error << "\n" << code;
    };

    for (const mx::NodeDefPtr& node_def : document->getNodeDefs())
        expect_compiles(node_def->getName(), decompiler.decompile_node_def(node_def, /*with_dependencies*/true));

    for (const mx::NodeGraphPtr& node_graph : document->getNodeGraphs())
    {
        if (node_graph->getNodeDef() == nullptr)
            expect_compiles(node_graph->getName(), decompiler.decompile_node_graph(node_graph, /*with_dependencies*/true));
    }

    for (const mx::NodePtr& node : document->getNodes())
        expect_compiles(node->getName(), decompiler.decompile_node(node, /*with_dependencies*/true));
}

namespace
{
    vector<fs::path> get_dependency_files()
    {
        const fs::path test_dir = get_test_data("decompile_dependencies");

        if (not fs::exists(test_dir))
            return {};

        vector<fs::path> files;
        for (const auto& p : fs::directory_iterator(test_dir))
            if (p.path().extension() == ".mtlx")
                files.push_back(p.path());
        std::sort(files.begin(), files.end());

        return files;
    }
}

INSTANTIATE_TEST_SUITE_P(
    decompiler,
    decompile_dependencies_tests,
    testing::ValuesIn(get_dependency_files()),
    [](const testing::TestParamInfo<fs::path>& info) {
        return info.param.stem().string();
    }
);
