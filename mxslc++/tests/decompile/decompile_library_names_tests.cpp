//
// Created by jaket on 13/11/2026.
//

// The node defs, node graphs and nodes of a document can have the names of elements of the MaterialX libraries, e.g.,
// when it is written by hand or by another tool, and MaterialX finds the library elements first. The decompiler must
// still decompile the elements of the document. The compiler gives its elements other names, see
// data/groundtruth/misc/library_names001.mxsl.

#include "gtest/gtest.h"
#include <string>
#include "decompile/Decompiler.h"
#include "utils/comp_utils.h"

using std::string;

namespace
{
    // NG_standard_surface_surfaceshader_100 is also named by an implementation element of the libraries
    const string DOCUMENT = R"(<?xml version="1.0"?>
        <materialx version="1.39">
          <nodedef name="ND_invert_float" node="invert_float">
            <output name="out" type="float" />
            <input name="in" type="float" value="0" />
          </nodedef>
          <nodegraph name="NG_invert_float" nodedef="ND_invert_float">
            <multiply name="result" type="float">
              <input name="in1" type="float" interfacename="in" />
              <input name="in2" type="float" value="-1" />
            </multiply>
            <output name="out" type="float" nodename="result" />
          </nodegraph>
          <nodegraph name="NG_tiledimage_float">
            <randomfloat name="b" type="float" />
            <output name="out" type="float" nodename="b" />
          </nodegraph>
          <nodegraph name="NG_standard_surface_surfaceshader_100">
            <randomcolor name="c" type="color3" />
            <output name="out" type="color3" nodename="c" />
          </nodegraph>
          <invert_float name="y" type="float">
            <input name="in" type="float" value="0.25" />
          </invert_float>
          <add name="w" type="float">
            <input name="in1" type="float" nodegraph="NG_tiledimage_float" />
            <input name="in2" type="float" nodename="y" />
          </add>
        </materialx>
    )";

    const string INVERT_FLOAT = "[[nodedef]] float invert_float(float in) { float result = in * -1.0; return result; }";
    const string TILEDIMAGE_FLOAT = "float tiledimage_float => { float b = randomfloat(); return b; }";
    const string STANDARD_SURFACE = "color3 standard_surface_surfaceshader_100 => { color3 c = randomcolor(); return c; }";
}

TEST(decompile_library_names_tests, document_is_decompiled)
{
    mxslc::decompile::Decompiler decompiler{DOCUMENT};
    const string expected = INVERT_FLOAT + TILEDIMAGE_FLOAT + STANDARD_SURFACE + "float y = invert_float(0.25); float w = tiledimage_float + y;";
    EXPECT_EQ(get_code_tokens(decompiler.decompile_document()), get_code_tokens(expected));
}

TEST(decompile_library_names_tests, node_def_is_decompiled_by_name)
{
    mxslc::decompile::Decompiler decompiler{DOCUMENT};
    EXPECT_EQ(get_code_tokens(decompiler.decompile_node_def("ND_invert_float")), get_code_tokens(INVERT_FLOAT));
}

TEST(decompile_library_names_tests, node_graph_is_decompiled_by_name)
{
    mxslc::decompile::Decompiler decompiler{DOCUMENT};
    EXPECT_EQ(get_code_tokens(decompiler.decompile_node_graph("NG_tiledimage_float")), get_code_tokens(TILEDIMAGE_FLOAT));
}

TEST(decompile_library_names_tests, nodes_keep_their_connections)
{
    // UV0 is a geomprop and distance a unit type of the libraries
    mxslc::decompile::Decompiler decompiler{string{R"(<?xml version="1.0"?>
        <materialx version="1.39">
          <texcoord name="UV0" type="vector2" />
          <magnitude name="distance" type="float">
            <input name="in" type="vector2" nodename="UV0" />
          </magnitude>
          <multiply name="scaled" type="float">
            <input name="in1" type="float" nodename="distance" />
            <input name="in2" type="float" value="20" />
          </multiply>
        </materialx>
    )"}};
    const string expected = "vec2 UV0 = texcoord(); float distance = magnitude(UV0); float scaled = distance * 20.0;";
    EXPECT_EQ(get_code_tokens(decompiler.decompile_document()), get_code_tokens(expected));
    EXPECT_EQ(get_code_tokens(decompiler.decompile_node("distance")), get_code_tokens("float distance = magnitude(UV0);"));
}
