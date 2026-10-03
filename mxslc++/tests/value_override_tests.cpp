//
// Tests that assigning to a node input replaces whatever binding was there
// before, instead of leaving both on the port.
//

#include "gtest/gtest.h"
#include <string>

#include <MaterialXCore/Document.h>

#include "CompileOptions.h"
#include "compile.h"

using std::string;

namespace
{
    // base_color is bound to a function result first, then to a plain value.
    // Graph reduction is disabled so the function call stays a node, which means
    // the second assignment has to clear the node connection from the first.
    const string VALUE_AFTER_NODE_CONNECTION =
        "color3 add(\n"
        "    color3 v1 = {1,1,1},\n"
        "    color3 v2 = {1,1,1})\n"
        "{\n"
        "    return v1 + v2;\n"
        "}\n"
        "\n"
        "color3 c = {1,1,1};\n"
        "mutable surfaceshader surface = standard_surface();\n"
        "surface.base_color = add({2,2,2}, {1,1,1});\n"
        "surface.base_color = c;\n";

    // The same input, with the assignments swapped: a plain value first, then a
    // function result, so the node connection has to clear the value instead.
    const string NODE_CONNECTION_AFTER_VALUE =
        "color3 add(\n"
        "    color3 v1 = {1,1,1},\n"
        "    color3 v2 = {1,1,1})\n"
        "{\n"
        "    return v1 + v2;\n"
        "}\n"
        "\n"
        "color3 c = {1,1,1};\n"
        "mutable surfaceshader surface = standard_surface();\n"
        "surface.base_color = c;\n"
        "surface.base_color = add({2,2,2}, {1,1,1});\n";
}

TEST(value_override_tests, value_replaces_a_previous_node_connection)
{
    mxslc::CompileOptions opts;
    opts.reduce_graph = false;
    // Validation is off so a double binding shows up as the specific assertion
    // below, rather than as a generic "invalid document" exception.
    opts.validate_graph = false;

    const mx::DocumentPtr doc = mxslc::compile_to_document(VALUE_AFTER_NODE_CONNECTION, opts);

    const mx::NodePtr surface = doc->getNode("surface");
    ASSERT_TRUE(surface) << "No surface node in the compiled document.";

    const mx::InputPtr base_color = surface->getInput("base_color");
    ASSERT_TRUE(base_color) << "Base color input was not written.";

    EXPECT_TRUE(base_color->hasValue()) << "The assigned value is missing.";
    EXPECT_FALSE(base_color->hasNodeName()) << "A stale node connection was left on the input.";
    EXPECT_FALSE(base_color->hasNodeGraphString()) << "A stale node graph binding was left on the input.";
    EXPECT_FALSE(base_color->hasInterfaceName()) << "A stale interface binding was left on the input.";
}

TEST(value_override_tests, node_connection_replaces_a_previous_value)
{
    mxslc::CompileOptions opts;
    opts.reduce_graph = false;
    opts.validate_graph = false;

    const mx::DocumentPtr doc = mxslc::compile_to_document(NODE_CONNECTION_AFTER_VALUE, opts);

    const mx::NodePtr surface = doc->getNode("surface");
    ASSERT_TRUE(surface) << "No surface node in the compiled document.";

    const mx::InputPtr base_color = surface->getInput("base_color");
    ASSERT_TRUE(base_color) << "Base color input was not written.";

    EXPECT_FALSE(base_color->hasValue()) << "A stale value was left on the input.";
    EXPECT_TRUE(base_color->hasNodeName()) << "The node connection is missing.";
    EXPECT_FALSE(base_color->hasNodeGraphString()) << "A stale node graph binding was left on the input.";
    EXPECT_FALSE(base_color->hasInterfaceName()) << "A stale interface binding was left on the input.";
}

namespace
{
    // Graph reduction is off so function/node bindings stay references instead of
    // folding into literals. Validation is off so a double binding shows up as a
    // specific assertion rather than a generic "invalid document" exception.
    mx::DocumentPtr compile_unreduced(const string& source)
    {
        mxslc::CompileOptions opts;
        opts.reduce_graph = false;
        opts.validate_graph = false;
        return mxslc::compile_to_document(source, opts);
    }

    mx::InputPtr surface_input(const mx::DocumentPtr& doc, const string& input_name)
    {
        const mx::NodePtr surface = doc->getNode("surface");
        return surface ? surface->getInput(input_name) : nullptr;
    }

    // A [[nodegraph]] function call binds through `nodegraph`; a plain function
    // call binds through `nodename`.
    const string NODE_GRAPH_FUNCTION =
        "[[nodegraph]]\n"
        "color3 foo()\n"
        "{\n"
        "    return {1,1,1};\n"
        "}\n"
        "\n";
}

TEST(value_override_tests, value_replaces_a_previous_node_graph_binding)
{
    const string source = NODE_GRAPH_FUNCTION +
        "color3 c = {1,1,1};\n"
        "mutable surfaceshader surface = standard_surface();\n"
        "surface.base_color = foo();\n"
        "surface.base_color = c;\n";

    const mx::DocumentPtr doc = compile_unreduced(source);
    const mx::InputPtr base_color = surface_input(doc, "base_color");
    ASSERT_TRUE(base_color) << "Base color input was not written.";

    EXPECT_TRUE(base_color->hasValue()) << "The assigned value is missing.";
    EXPECT_FALSE(base_color->hasNodeGraphString()) << "A stale node graph binding was left on the input.";
    EXPECT_FALSE(base_color->hasNodeName()) << "A stale node connection was left on the input.";
    EXPECT_FALSE(base_color->hasInterfaceName()) << "A stale interface binding was left on the input.";
}

TEST(value_override_tests, node_graph_binding_replaces_a_previous_value)
{
    const string source = NODE_GRAPH_FUNCTION +
        "color3 c = {1,1,1};\n"
        "mutable surfaceshader surface = standard_surface();\n"
        "surface.base_color = c;\n"
        "surface.base_color = foo();\n";

    const mx::DocumentPtr doc = compile_unreduced(source);
    const mx::InputPtr base_color = surface_input(doc, "base_color");
    ASSERT_TRUE(base_color) << "Base color input was not written.";

    EXPECT_FALSE(base_color->hasValue()) << "A stale value was left on the input.";
    EXPECT_TRUE(base_color->hasNodeGraphString()) << "The node graph binding is missing.";
    EXPECT_FALSE(base_color->hasNodeName()) << "A stale node connection was left on the input.";
    EXPECT_FALSE(base_color->hasInterfaceName()) << "A stale interface binding was left on the input.";
}

TEST(value_override_tests, output_binding_replaces_a_previous_value)
{
    const string source =
        "float c = 0.5;\n"
        "mutable surfaceshader surface = standard_surface();\n"
        "surface.specular_roughness = c;\n"
        "surface.specular_roughness = separate3(in = vec3{1,2,3})[0];\n";

    const mx::DocumentPtr doc = compile_unreduced(source);
    const mx::InputPtr roughness = surface_input(doc, "specular_roughness");
    ASSERT_TRUE(roughness) << "Specular roughness input was not written.";

    EXPECT_FALSE(roughness->hasValue()) << "A stale value was left on the input.";
    EXPECT_TRUE(roughness->hasNodeName()) << "The node connection is missing.";
    EXPECT_TRUE(roughness->hasOutputString()) << "The connected output is missing.";
    EXPECT_FALSE(roughness->hasNodeGraphString()) << "A stale node graph binding was left on the input.";
    EXPECT_FALSE(roughness->hasInterfaceName()) << "A stale interface binding was left on the input.";
}

TEST(value_override_tests, value_replaces_a_previous_output_binding)
{
    const string source =
        "float c = 0.5;\n"
        "mutable surfaceshader surface = standard_surface();\n"
        "surface.specular_roughness = separate3(in = vec3{1,2,3})[0];\n"
        "surface.specular_roughness = c;\n";

    const mx::DocumentPtr doc = compile_unreduced(source);
    const mx::InputPtr roughness = surface_input(doc, "specular_roughness");
    ASSERT_TRUE(roughness) << "Specular roughness input was not written.";

    EXPECT_TRUE(roughness->hasValue()) << "The assigned value is missing.";
    EXPECT_FALSE(roughness->hasNodeName()) << "A stale node connection was left on the input.";
    EXPECT_FALSE(roughness->hasOutputString()) << "A stale connected output was left on the input.";
    EXPECT_FALSE(roughness->hasNodeGraphString()) << "A stale node graph binding was left on the input.";
    EXPECT_FALSE(roughness->hasInterfaceName()) << "A stale interface binding was left on the input.";
}

TEST(value_override_tests, default_value_replaces_a_previous_node_connection)
{
    const string source =
        "mutable surfaceshader surface = standard_surface();\n"
        "surface.specular_roughness = randomfloat();\n"
        "float x;\n"
        "surface.specular_roughness = x;\n";

    const mx::DocumentPtr doc = compile_unreduced(source);
    const mx::InputPtr roughness = surface_input(doc, "specular_roughness");
    ASSERT_TRUE(roughness) << "Specular roughness input was not written.";

    EXPECT_TRUE(roughness->hasValue()) << "The default value is missing.";
    EXPECT_FALSE(roughness->hasNodeName()) << "A stale node connection was left on the input.";
    EXPECT_FALSE(roughness->hasNodeGraphString()) << "A stale node graph binding was left on the input.";
    EXPECT_FALSE(roughness->hasInterfaceName()) << "A stale interface binding was left on the input.";
}

TEST(value_override_tests, default_value_replaces_a_previous_output_binding)
{
    const string source =
        "mutable surfaceshader surface = standard_surface();\n"
        "surface.specular_roughness = separate3(in = vec3{1,2,3})[0];\n"
        "float x;\n"
        "surface.specular_roughness = x;\n";

    const mx::DocumentPtr doc = compile_unreduced(source);
    const mx::InputPtr roughness = surface_input(doc, "specular_roughness");
    ASSERT_TRUE(roughness) << "Specular roughness input was not written.";

    EXPECT_TRUE(roughness->hasValue()) << "The default value is missing.";
    EXPECT_FALSE(roughness->hasNodeName()) << "A stale node connection was left on the input.";
    EXPECT_FALSE(roughness->hasOutputString()) << "A stale connected output was left on the input.";
    EXPECT_FALSE(roughness->hasNodeGraphString()) << "A stale node graph binding was left on the input.";
    EXPECT_FALSE(roughness->hasInterfaceName()) << "A stale interface binding was left on the input.";
}
