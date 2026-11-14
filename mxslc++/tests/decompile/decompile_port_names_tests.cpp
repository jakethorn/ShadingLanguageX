//
// Created by jaket on 13/11/2026.
//

// The ports of node defs can have names that are not identifiers, e.g., keywords, which the parameters of functions are
// named after, e.g., if_ for the input if. Calls to the function must name their arguments in the same way, but calls to
// library functions use the names of their ports, e.g., `image("a.png", default = c)`. The compiler names the ports after
// the parameters, so they are renamed by the round trip. The compiler also names the outputs of a function after its
// return value, i.e., out, or out__<field> for the fields of a struct, so the attributes of outputs must name them so.

#include "gtest/gtest.h"
#include <string>
#include "compile.h"
#include "CompileOptions.h"
#include "decompile/Decompiler.h"
#include "utils/comp_utils.h"

using std::string;

namespace
{
    const string KEYWORD_PORTS_DOCUMENT = R"(<?xml version="1.0"?>
        <materialx version="1.39">
          <nodedef name="ND_kw" node="kw">
            <input name="in" type="float" value="2" />
            <input name="default" type="float" value="1" />
            <input name="if" type="float" value="3" />
            <output name="out" type="float" />
          </nodedef>
          <nodegraph name="NG_kw" nodedef="ND_kw">
            <add name="a" type="float">
              <input name="in1" type="float" interfacename="default" />
              <input name="in2" type="float" interfacename="if" />
            </add>
            <output name="out" type="float" nodename="a" />
          </nodegraph>
          <kw name="x" type="float">
            <input name="default" type="float" value="5" />
            <input name="if" type="float" value="6" />
          </kw>
          <image name="c" type="color3">
            <input name="file" type="filename" value="a.png" />
            <input name="default" type="color3" value="1, 0, 0" />
          </image>
        </materialx>
    )";

    const string OUTPUT_ATTRIBUTES_DOCUMENT = R"(<?xml version="1.0"?>
        <materialx version="1.39">
          <nodedef name="ND_split" node="split">
            <input name="in" type="float" value="0" />
            <output name="outx" type="float" default="0.0" />
            <output name="outy" type="float" doc="three times in" />
          </nodedef>
          <nodegraph name="NG_split" nodedef="ND_split">
            <multiply name="a" type="float">
              <input name="in1" type="float" interfacename="in" />
              <input name="in2" type="float" value="2" />
            </multiply>
            <multiply name="b" type="float">
              <input name="in1" type="float" interfacename="in" />
              <input name="in2" type="float" value="3" />
            </multiply>
            <output name="outx" type="float" nodename="a" />
            <output name="outy" type="float" nodename="b" />
          </nodegraph>
          <nodedef name="ND_half" node="half">
            <input name="in" type="float" value="0" />
            <output name="result" type="float" doc="half of in" />
          </nodedef>
          <nodegraph name="NG_half" nodedef="ND_half">
            <multiply name="h" type="float">
              <input name="in1" type="float" interfacename="in" />
              <input name="in2" type="float" value="0.5" />
            </multiply>
            <output name="result" type="float" nodename="h" />
          </nodegraph>
        </materialx>
    )";
}

TEST(decompile_port_names_tests, arguments_are_named_like_the_parameters)
{
    mxslc::decompile::Decompiler decompiler{KEYWORD_PORTS_DOCUMENT};
    const string code = decompiler.decompile_document();
    const string expected = R"(
        [[nodedef]] float kw(float in = 2.0, float default_ = 1.0, float if_ = 3.0) { float a = default_ + if_; return a; }
        float x = kw(default_ = 5.0, if_ = 6.0);
        color3 c = image("a.png", default = color3{1.0, 0.0, 0.0});
    )";
    EXPECT_EQ(get_code_tokens(code), get_code_tokens(expected));

    mxslc::CompileOptions opts;
    opts.reduce_graph = false;
    EXPECT_NO_THROW(mxslc::compile_to_document(code, opts)) << code;
}

TEST(decompile_port_names_tests, output_attributes_name_the_compiled_outputs)
{
    mxslc::decompile::Decompiler decompiler{OUTPUT_ATTRIBUTES_DOCUMENT};
    const string code = decompiler.decompile_document();
    const string expected = R"(
        @out__outx.default "0.0" @out__outy.doc "three times in"
        [[nodedef]] {float outx, float outy} split(float in) { float a = in * 2.0; float b = in * 3.0; return {a, b}; }
        @out.doc "half of in"
        [[nodedef]] float half(float in) { float h = in * 0.5; return h; }
    )";
    EXPECT_EQ(get_code_tokens(code), get_code_tokens(expected));

    mxslc::CompileOptions opts;
    opts.reduce_graph = false;
    EXPECT_NO_THROW(mxslc::compile_to_document(code, opts)) << code;
}
