//
// Created by jaket on 31/10/2026.
//

#include "gtest/gtest.h"

#include "common.h"
#include "CompileOptions.h"
#include "compile.h"
#include "errors/CompileError.h"

using namespace mxslc;

namespace
{
    const string twice_source = R"(
float twice(float x) { return x * 2.0; }
float y = twice(texcoord<vec2>().x);
)";

    string compile_with(const string& source, const FunctionBehaviour behaviour, const bool reduce_graph = true)
    {
        CompileOptions opts;
        opts.default_function_behaviour = behaviour;
        opts.reduce_graph = reduce_graph;
        return compile_to_string(source, opts);
    }

    size_t count_occurrences(const string& xml, const string& text)
    {
        size_t count = 0;
        for (size_t i = xml.find(text); i != string::npos; i = xml.find(text, i + text.size()))
            ++count;
        return count;
    }
}

TEST(default_function_behaviour_tests, test_nodedef_is_the_default)
{
    const string xml = compile_to_string(twice_source);

    EXPECT_EQ(count_occurrences(xml, "<nodedef name=\"ND_twice\""), 1);
    EXPECT_EQ(count_occurrences(xml, "<nodegraph name=\"NG_twice\""), 1);
}

TEST(default_function_behaviour_tests, test_inline)
{
    const string xml = compile_with(twice_source, FunctionBehaviour::Inline);

    EXPECT_EQ(count_occurrences(xml, "<nodedef"), 0);
    EXPECT_EQ(count_occurrences(xml, "<nodegraph"), 0);
    EXPECT_EQ(count_occurrences(xml, "<multiply"), 1);
}

TEST(default_function_behaviour_tests, test_inline_parameter_can_be_a_loop_bound)
{
    const string source = R"(
float count_up(int n) { mutable float s = 0.0; for (int i from 1 to n) { s += 1.0; } return s; }
float y = count_up(3) + texcoord<vec2>().x;
)";

    EXPECT_THROW(compile_with(source, FunctionBehaviour::NodeDef), CompileError);
    EXPECT_NO_THROW(compile_with(source, FunctionBehaviour::Inline));
}

TEST(default_function_behaviour_tests, test_comptime_folds_without_reduce_graph)
{
    const string source = R"(
float twice(float x) { return x * 2.0; }
float y = twice(3.0) + texcoord<vec2>().x;
)";

    EXPECT_EQ(count_occurrences(compile_with(source, FunctionBehaviour::Inline, false), "<multiply"), 1);
    EXPECT_EQ(count_occurrences(compile_with(source, FunctionBehaviour::Comptime, false), "<multiply"), 0);
}

TEST(default_function_behaviour_tests, test_nodegraph)
{
    const string source = R"(
float twice(float x = 1.0) { return x * 2.0; }
float y = twice() + texcoord<vec2>().x;
)";

    const string xml = compile_with(source, FunctionBehaviour::NodeGraph);

    EXPECT_EQ(count_occurrences(xml, "<nodedef"), 0);
    EXPECT_EQ(count_occurrences(xml, "<nodegraph name=\"NG_twice\""), 1);
    EXPECT_THROW(compile_with(twice_source, FunctionBehaviour::NodeGraph), CompileError);
}

TEST(default_function_behaviour_tests, test_function_modifiers_override_the_default)
{
    const string source = R"(
nodedef float twice(float x) { return x * 2.0; }
inline float thrice(float x) { return x * 3.0; }
float y = twice(texcoord<vec2>().x) + thrice(texcoord<vec2>().y);
)";

    for (const FunctionBehaviour behaviour : {FunctionBehaviour::NodeDef, FunctionBehaviour::Inline})
    {
        const string xml = compile_with(source, behaviour);
        EXPECT_EQ(count_occurrences(xml, "<nodedef name=\"ND_twice\""), 1);
        EXPECT_EQ(count_occurrences(xml, "<nodedef name=\"ND_thrice\""), 0);
    }
}

TEST(default_function_behaviour_tests, test_constructors_use_the_default)
{
    const string source = R"(
class Ring { float r; Ring(float r) { this.r = r; } }
Ring ring = Ring{texcoord<vec2>().x};
float y = ring.r;
)";

    EXPECT_EQ(count_occurrences(compile_with(source, FunctionBehaviour::Inline), "<nodedef"), 0);
}
