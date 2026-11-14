//
// Created by jaket on 13/11/2026.
//

// The Decompiler decompiles the elements of its document, which are given by name, or as elements of the document or of
// the decompiler's copy of it, see Decompiler::document. Elements of other documents are rejected, even if the document
// has an element with the same path, e.g., the MaterialX libraries can have a node def with the name of a node. A node
// that is decompiled with its dependencies also needs the declarations of the variables that its values are assigned to.
// A string is MaterialX source if it starts with an element, otherwise it is the path of a file.

#include "gtest/gtest.h"
#include <fstream>
#include <functional>
#include <sstream>
#include <string>
#include "compile.h"
#include "CompileOptions.h"
#include "decompile/Decompiler.h"
#include "errors/CompileError.h"
#include "utils/comp_utils.h"
#include "utils/data_utils.h"

using std::string;

namespace
{
    // a node with the name of a node def of the MaterialX libraries
    const string DOCUMENT = R"(<?xml version="1.0"?>
        <materialx version="1.39">
          <add name="ND_add_float" type="float">
            <input name="in1" type="float" value="1" />
          </add>
        </materialx>
    )";

    const string FOO = "[[nodedef]] float foo(float x) { return x + 1.0; }";

    // a is declared by its node, a = position(), and b by the first value that is assigned to it, b = tangent(), because
    // its definition is a compile-time value
    const string REASSIGNED_VARIABLES = R"(
        mutable vec3 a = position();
        float x = a[0];
        a = normal();
        float i = a[1];
        mutable vec3 b = vec3{1.0, 2.0, 3.0};
        b = tangent();
        b *= 2.0;
        b = bitangent();
        float n = magnitude(b);
    )";

    mx::DocumentPtr compile(const string& code)
    {
        mxslc::CompileOptions opts;
        opts.reduce_graph = false;
        return mxslc::compile_to_document(code, opts);
    }

    // the message of the error that the function throws, or nothing if it does not throw
    string get_error_message(const std::function<void()>& function)
    {
        try
        {
            function();
            return "";
        }
        catch (const mxslc::CompileError& e)
        {
            return e.message();
        }
    }
}

TEST(decompiler_api_tests, elements_of_the_document_and_of_its_copy_are_decompiled)
{
    const mx::DocumentPtr document = compile(FOO);
    mxslc::decompile::Decompiler decompiler{document};
    EXPECT_EQ(get_code_tokens(decompiler.decompile_node_def(document->getNodeDef("ND_foo"))), get_code_tokens(FOO));
    EXPECT_EQ(get_code_tokens(decompiler.decompile_node_def(decompiler.document()->getNodeDef("ND_foo"))), get_code_tokens(FOO));
}

TEST(decompiler_api_tests, library_elements_are_rejected)
{
    mxslc::decompile::Decompiler decompiler{DOCUMENT};
    // the copy of the document has the MaterialX libraries as its data library, which MaterialX looks in first
    const mx::NodeDefPtr node_def = decompiler.document()->getNodeDef("ND_add_float");
    EXPECT_EQ(get_error_message([&] { decompiler.decompile_node_def(node_def); }), "ND_add_float is not an element of the decompiled document");
}

TEST(decompiler_api_tests, elements_of_other_documents_are_rejected)
{
    mxslc::decompile::Decompiler decompiler{compile(FOO)};
    const mx::DocumentPtr other = compile("float foo(float x) { return x * 100.0; }");
    EXPECT_EQ(get_error_message([&] { decompiler.decompile_node_def(other->getNodeDef("ND_foo")); }), "ND_foo is not an element of the decompiled document");
}

TEST(decompiler_api_tests, missing_elements_are_named)
{
    mxslc::decompile::Decompiler decompiler{DOCUMENT};
    EXPECT_EQ(get_error_message([&] { decompiler.decompile_node("x"); }), "The document has no node named 'x'");
    EXPECT_EQ(get_error_message([&] { decompiler.decompile_node_def("ND_add_float"); }), "The document has no node def named 'ND_add_float'");
    EXPECT_EQ(get_error_message([&] { decompiler.decompile_node_graph("NG_tiledimage_float"); }), "The document has no node graph named 'NG_tiledimage_float'");
}

TEST(decompiler_api_tests, nodes_of_node_graphs_and_node_defs_without_node_graphs_are_rejected)
{
    const mx::DocumentPtr document = compile(FOO);
    document->addNodeDef("ND_nobody", "float", "nobody");
    mxslc::decompile::Decompiler decompiler{document};

    const mx::NodePtr node = document->getNodeGraph("NG_foo")->getNodes().front();
    EXPECT_EQ(get_error_message([&] { decompiler.decompile_node(node); }), "Only top-level nodes can be passed to decompile_node, but NG_foo/" + node->getName() + " is in the node graph NG_foo");
    EXPECT_EQ(get_error_message([&] { decompiler.decompile_node_def("ND_nobody"); }), "The node graph of the node def ND_nobody could not be found");
}

TEST(decompiler_api_tests, dependencies_declare_the_variables_that_values_are_assigned_to)
{
    mxslc::decompile::Decompiler decompiler{compile(REASSIGNED_VARIABLES)};
    const string i = decompiler.decompile_node("i", true);
    EXPECT_EQ(get_code_tokens(i), get_code_tokens("mutable vec3 a = position(); a = normal(); float i = a.y;"));
    EXPECT_NO_THROW(compile(i)) << i;

    const string n = decompiler.decompile_node("n", true);
    EXPECT_EQ(get_code_tokens(n), get_code_tokens("mutable vec3 b = tangent(); b = bitangent(); float n = magnitude(b);"));
    EXPECT_NO_THROW(compile(n)) << n;
}

TEST(decompiler_api_tests, strings_are_materialx_source_or_the_path_of_a_file)
{
    const fs::path path = get_test_data("decompile_cases/matrix_constants.mtlx");
    const string expected = mxslc::decompile::Decompiler{path}.decompile_document();
    EXPECT_EQ(mxslc::decompile::Decompiler{path.string()}.decompile_document(), expected);

    // the source can start with whitespace and a byte order mark
    std::ifstream file{path};
    std::stringstream source;
    source << "\xEF\xBB\xBF \n" << file.rdbuf();
    EXPECT_EQ(mxslc::decompile::Decompiler{source.str()}.decompile_document(), expected);
}
