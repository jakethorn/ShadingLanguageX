//
// Tests for the cache of loaded MaterialX libraries (load_materialx_library),
// which is shared between compiles so the libraries are only parsed once.
//

#include "gtest/gtest.h"
#include <string>

#include "CompileOptions.h"
#include "compile.h"
#include "decompile/decompile.h"
#include "utils/io_utils.h"
#include "utils/load_mtlx.h"

using std::string;

namespace
{
    const string SOURCE =
        "vec3 n = normal();\n"
        "surfaceshader surface = standard_surface(0.5);\n"
        "surface.base_color = randomcolor();\n"
        "surface.specular_roughness = 0.25;\n";
}

TEST(materialx_library_cache, same_arguments_return_the_same_document)
{
    const auto dirs = mxslc::io_utils::get_default_search_directories();

    const mx::DocumentPtr first = mxslc::load_materialx_library("1.39", dirs);
    const mx::DocumentPtr second = mxslc::load_materialx_library("1.39", dirs);

    EXPECT_EQ(first, second);
    EXPECT_FALSE(first->getNodeDefs().empty());
}

TEST(materialx_library_cache, different_versions_return_different_documents)
{
    const auto dirs = mxslc::io_utils::get_default_search_directories();

    const mx::DocumentPtr v138 = mxslc::load_materialx_library("1.38", dirs);
    const mx::DocumentPtr v139 = mxslc::load_materialx_library("1.39", dirs);

    EXPECT_NE(v138, v139);
}

TEST(materialx_library_cache, compiles_do_not_modify_the_shared_library)
{
    mxslc::CompileOptions opts;
    opts.add_default_search_directories();
    const mx::DocumentPtr library = mxslc::load_materialx_library(opts.version, opts.search_directories());
    const size_t child_count = library->getChildren().size();

    const string first = mxslc::compile_to_string(SOURCE);
    const string second = mxslc::compile_to_string(SOURCE);
    mxslc::decompile::decompile_to_string(first);

    EXPECT_EQ(first, second);
    EXPECT_EQ(library, mxslc::load_materialx_library(opts.version, opts.search_directories()));
    EXPECT_EQ(library->getChildren().size(), child_count);
}
