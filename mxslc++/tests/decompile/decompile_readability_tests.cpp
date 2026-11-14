//
// Created by jaket on 14/11/2026.
//

// Forms of code that the decompiler writes because they read better and compile to the same graph: parameters returned
// without the dot nodes that the compiler passes them through, temporary separate nodes written as destructurings, and
// parameters whose attributes do not fit on their line written after them.

#include "gtest/gtest.h"
#include <string>
#include "compile.h"
#include "CompileOptions.h"
#include "decompile/Decompiler.h"
#include "utils/comp_utils.h"

using std::string;

namespace
{
    mx::DocumentPtr compile(const string& code)
    {
        mxslc::CompileOptions opts;
        opts.reduce_graph = false;
        return mxslc::compile_to_document(code, opts);
    }

    string decompile(const string& code)
    {
        mxslc::decompile::Decompiler decompiler{compile(code)};
        return decompiler.decompile_document();
    }
}

TEST(decompile_readability_tests, parameters_are_returned_without_their_dot_nodes)
{
    // the dot node of y keeps the value of x from before x is assigned to
    const string code = decompile(R"(
        [[nodedef]] int f(int a) { return a; }
        [[nodedef]] void g(ref float x, ref float y) { y = x; x = 5.0; }
    )");
    const string expected = R"(
        [[nodedef]] int f(int a) { return a; }
        [[nodedef]] void g(ref float x, ref float y) { float node2 = dot(x); x = 5.0; y = node2; }
    )";
    EXPECT_EQ(get_code_tokens(code), get_code_tokens(expected));
    EXPECT_NO_THROW(compile(code)) << code;
}

TEST(decompile_readability_tests, temporary_separate_nodes_are_destructured)
{
    // separate nodes with a name keep it
    const string code = decompile(R"(
        vec2 v = texcoord();
        float a, b = v;
        float c = a + b;
        {float outx, float outy} s = separate2(v);
        float d = s.outx * 2.0;
    )");
    const string expected = R"(
        vec2 v = texcoord();
        float v_x, v_y = v;
        float c = v_x + v_y;
        {float outx, float outy} s = separate2(v);
        float d = s.outx * 2.0;
    )";
    EXPECT_EQ(get_code_tokens(code), get_code_tokens(expected));
    EXPECT_NO_THROW(compile(code)) << code;
}

TEST(decompile_readability_tests, parameters_that_do_not_fit_are_written_after_their_attributes)
{
    const string document = R"(<?xml version="1.0"?>
        <materialx version="1.39">
          <nodedef name="ND_tint" node="tint">
            <input name="base_color" type="color3" value="0.8, 0.8, 0.8" uimin="0,0,0" uimax="1,1,1" uiname="Base Color" uifolder="Base" doc="Color of the diffuse reflection." />
            <input name="amount" type="float" value="0.5" uiname="Amount" />
            <output name="out" type="color3" />
          </nodedef>
          <nodegraph name="NG_tint" nodedef="ND_tint">
            <multiply name="m" type="color3">
              <input name="in1" type="color3" interfacename="base_color" />
              <input name="in2" type="float" interfacename="amount" />
            </multiply>
            <output name="out" type="color3" nodename="m" />
          </nodegraph>
        </materialx>
    )";
    mxslc::decompile::Decompiler decompiler{document};
    const string expected =
        "[[nodedef]]\n"
        "color3 tint(\n"
        "@uimin \"0,0,0\" @uimax \"1,1,1\" @uiname \"Base Color\" @uifolder \"Base\"\n"
        "@doc \"Color of the diffuse reflection.\"\n"
        "color3 base_color = color3{0.8},\n"
        "@uiname \"Amount\" float amount = 0.5)\n"
        "{\n"
        "    color3 m = base_color * amount;\n"
        "    return m;\n"
        "}\n";
    const string code = decompiler.decompile_document();
    EXPECT_EQ(code, expected);
    EXPECT_NO_THROW(compile(code)) << code;
}
