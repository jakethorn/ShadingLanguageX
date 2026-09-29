//
// Tests for DecompileOptions and the `[[nodegraph]]` / `[[nodedef]]` function
// modifiers emitted for MTLX nodegraphs and nodedefs.
//

#include "gtest/gtest.h"
#include <string>

#include "compile.h"
#include "decompile/decompile.h"
#include "decompile/DecompileOptions.h"

using std::string;

namespace
{
    using mxslc::decompile::DecompileOptions;

    // Compound nodegraph test.
    const string NODE_GRAPH_MTLX =
        "<?xml version=\"1.0\"?>\n"
        "<materialx version=\"1.39\">\n"
        "  <nodegraph name=\"NG_scale\">\n"
        "    <input name=\"x\" type=\"float\" value=\"1.0\" />\n"
        "    <multiply name=\"m\" type=\"float\">\n"
        "      <input name=\"in1\" type=\"float\" interfacename=\"x\" />\n"
        "      <input name=\"in2\" type=\"float\" value=\"2.0\" />\n"
        "    </multiply>\n"
        "    <output name=\"out\" type=\"float\" nodename=\"m\" />\n"
        "  </nodegraph>\n"
        "</materialx>\n";

    // Definition test which also tests attribute vs tag ordering.
    const string NODE_DEF_MTLX =
        "<?xml version=\"1.0\"?>\n"
        "<materialx version=\"1.39\">\n"
        "  <nodedef name=\"ND_fwidth\" node=\"fwidth\" nodegroup=\"math\">\n"
        "    <output name=\"out\" type=\"float\" />\n"
        "    <input name=\"value\" type=\"float\" value=\"0\" />\n"
        "  </nodedef>\n"
        "  <nodegraph name=\"NG_fwidth\" nodedef=\"ND_fwidth\">\n"
        "    <output name=\"out\" type=\"float\" interfacename=\"value\" />\n"
        "  </nodegraph>\n"
        "</materialx>\n";

    string decompile(const string& mtlx, const DecompileOptions& options)
    {
        return mxslc::decompile::decompile_to_string(mtlx, options);
    }
}

TEST(decompile_options_tests, modifiers_are_off_by_default)
{
    // Nodegraphs omit the modifiers by default.
    const string node_graph = mxslc::decompile::decompile_to_string(NODE_GRAPH_MTLX);
    EXPECT_EQ(node_graph.find("[[nodegraph]]"), string::npos) << node_graph;
    EXPECT_EQ(node_graph.find("[[nodedef]]"), string::npos) << node_graph;

    const string node_def = mxslc::decompile::decompile_to_string(NODE_DEF_MTLX);
    EXPECT_EQ(node_def.find("[[nodegraph]]"), string::npos) << node_def;
    EXPECT_EQ(node_def.find("[[nodedef]]"), string::npos) << node_def;
}

TEST(decompile_options_tests, modifiers_are_emitted_when_enabled)
{
    DecompileOptions options;
    options.emit_function_modifiers = true;

    // Nodegraphs emit `[[nodegraph]]`,  `[[nodedef]]`.
    const string node_graph = decompile(NODE_GRAPH_MTLX, options);
    EXPECT_NE(node_graph.find("[[nodegraph]]"), string::npos) << node_graph;
    EXPECT_EQ(node_graph.find("[[nodedef]]"), string::npos) << node_graph;

    const string node_def = decompile(NODE_DEF_MTLX, options);
    EXPECT_NE(node_def.find("[[nodedef]]"), string::npos) << node_def;
    EXPECT_EQ(node_def.find("[[nodegraph]]"), string::npos) << node_def;

    // Attributes are parsed before modifiers, so `@nodegroup` must come first.
    const size_t attr_pos = node_def.find("@nodegroup \"math\"");
    const size_t modifier_pos = node_def.find("[[nodedef]]");
    ASSERT_NE(attr_pos, string::npos) << node_def;
    ASSERT_NE(modifier_pos, string::npos) << node_def;
    EXPECT_LT(attr_pos, modifier_pos) << node_def;

    // The interface input is emitted as a (defaulted) parameter, but a
    // `[[nodegraph]]` function cannot be passed arguments, so the invocation
    // must be argument-less.
    EXPECT_NE(node_graph.find("scale(float x"), string::npos) << node_graph;
    EXPECT_NE(node_graph.find("= scale();"), string::npos) << node_graph;

    const string mtlx = mxslc::compile_to_string(node_graph);
    EXPECT_NE(mtlx.find("<materialx"), string::npos) << mtlx;
    EXPECT_NE(mtlx.find("nodegraph"), string::npos) << mtlx;
}
