//
// Created by jaket on 14/11/2026.
//

// ShadingLanguageX cannot express some elements of a document, e.g., looks, collections, outputs of the document, or node
// defs without a node graph, so they are not decompiled, but a comment above the code names them.

#include "gtest/gtest.h"
#include <string>
#include "compile.h"
#include "CompileOptions.h"
#include "decompile/Decompiler.h"

using std::string;

namespace
{
    const string DOCUMENT = R"(<?xml version="1.0"?>
        <materialx version="1.39">
          <nodedef name="ND_nobody" node="nobody">
            <output name="out" type="float" />
          </nodedef>
          <implementation name="IM_nobody_genglsl" nodedef="ND_nobody" target="genglsl" file="nobody.glsl" />
          <standard_surface name="ss" type="surfaceshader">
            <input name="base" type="float" value="0.5" />
          </standard_surface>
          <surfacematerial name="mat" type="material">
            <input name="surfaceshader" type="surfaceshader" nodename="ss" />
          </surfacematerial>
          <look name="lookA">
            <materialassign name="ma" material="mat" geom="/a/b" />
          </look>
          <collection name="col" includegeom="/a/*" />
          <typedef name="mystruct" />
          <token name="tok" type="string" value="x" />
          <output name="doc_out" type="material" nodename="mat" />
          <!-- a comment -->
        </materialx>
    )";
}

TEST(decompile_dropped_elements_tests, elements_that_are_not_decompiled_are_named)
{
    mxslc::decompile::Decompiler decompiler{DOCUMENT};
    const string code = decompiler.decompile_document();
    const string comment =
        "// Not decompiled, because ShadingLanguageX has no equivalent:\n"
        "//   nodedef: ND_nobody\n"
        "//   implementation: IM_nobody_genglsl\n"
        "//   look: lookA\n"
        "//   collection: col\n"
        "//   typedef: mystruct\n"
        "//   token: tok\n"
        "//   output: doc_out\n"
        "\n"
        "surfaceshader ss = standard_surface(0.5);\n"
        "material mat = surfacematerial(ss);";
    EXPECT_EQ(code.substr(0, comment.size()), comment) << code;

    mxslc::CompileOptions opts;
    opts.reduce_graph = false;
    EXPECT_NO_THROW(mxslc::compile_to_document(code, opts)) << code;
}
