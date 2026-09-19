//
// Tests for CompileOptions in-memory sources (CompileOptions::add_source),
// which let a multi-file project be compiled without staging it to disk.
//

#include "gtest/gtest.h"
#include <string>

#include "CompileOptions.h"
#include "compile.h"
#include "errors/CompileError.h"
#include "utils/data_utils.h"

using std::string;

namespace
{
    const string ROOT =
        "#include \"a.mxsl\"\n"
        "float z = value + 1.0;\n";
}

TEST(compile_options_sources, include_resolves_from_a_search_directory)
{
    mxslc::CompileOptions opts;
    opts.add_search_directory(get_test_data_dir());

    const string source =
        "#include \"includes/b.mxsl\"\n"
        "float z = value + 1.0;\n";

    EXPECT_NE(mxslc::compile_to_string(source, opts).find("<materialx"), string::npos);
}

TEST(compile_options_sources, include_resolves_from_an_in_memory_source)
{
    mxslc::CompileOptions opts;
    opts.add_source("a.mxsl", "float value = 12;");

    EXPECT_NE(mxslc::compile_to_string(ROOT, opts).find("<materialx"), string::npos);
}

TEST(compile_options_sources, nested_includes_resolve_from_in_memory_sources)
{
    mxslc::CompileOptions opts;
    opts.add_source("a.mxsl", "#include \"b.mxsl\"\nfloat value = base + base;");
    opts.add_source("b.mxsl", "float base = 6;");

    EXPECT_NE(mxslc::compile_to_string(ROOT, opts).find("<materialx"), string::npos);
}

TEST(compile_options_sources, in_memory_source_takes_precedence_over_a_file_on_disk)
{
    // data/includes/b.mxsl defines `value`, but the in-memory source below
    // defines a different name instead, so the compile only succeeds if the
    // in-memory source shadowed the file on disk.
    mxslc::CompileOptions opts;
    opts.add_search_directory(get_test_data_dir());
    opts.add_source("includes/b.mxsl", "float shadowed = 1.0;");

    const string source =
        "#include \"includes/b.mxsl\"\n"
        "float z = shadowed + 1.0;\n";

    EXPECT_NE(mxslc::compile_to_string(source, opts).find("<materialx"), string::npos);
}

TEST(compile_options_sources, unsupplied_include_still_fails)
{
    const mxslc::CompileOptions opts;

    EXPECT_THROW(mxslc::compile_to_string(ROOT, opts), mxslc::CompileError);
}

TEST(compile_options_sources, clearing_sources_removes_them)
{
    mxslc::CompileOptions opts;
    opts.add_source("a.mxsl", "float value = 12;");
    ASSERT_TRUE(opts.has_source("a.mxsl"));

    opts.clear_sources();

    EXPECT_FALSE(opts.has_source("a.mxsl"));
    EXPECT_THROW(mxslc::compile_to_string(ROOT, opts), mxslc::CompileError);
}

TEST(compile_options_sources, self_contained_compile_is_unaffected)
{
    const mxslc::CompileOptions opts;
    const string source = "float z = add(1.0, 2.0);";

    EXPECT_NE(mxslc::compile_to_string(source, opts).find("<materialx"), string::npos);
}

TEST(compile_options_sources, nested_include_resolves_relative_to_the_including_source)
{
    // sub/a.mxsl includes "b.mxsl", which must resolve to sub/b.mxsl rather
    // than to a file of that name at the top of the overlay.
    mxslc::CompileOptions opts;
    opts.reduce_graph = false;
    opts.add_source("sub/a.mxsl", "#include \"b.mxsl\"\nfloat value = base + base;");
    opts.add_source("sub/b.mxsl", "float base = 6;");

    const string source =
        "#include \"sub/a.mxsl\"\n"
        "float z = value + 1.0;\n";

    EXPECT_NE(mxslc::compile_to_string(source, opts).find("name=\"z\""), string::npos);
}

TEST(compile_options_sources, include_can_climb_out_of_a_subdirectory)
{
    // sub/a.mxsl includes "../shared/b.mxsl", which normalises to the
    // shared/b.mxsl key.
    mxslc::CompileOptions opts;
    opts.reduce_graph = false;
    opts.add_source("sub/a.mxsl", "#include \"../shared/b.mxsl\"\nfloat value = base + base;");
    opts.add_source("shared/b.mxsl", "float base = 6;");

    const string source =
        "#include \"sub/a.mxsl\"\n"
        "float z = value + 1.0;\n";

    EXPECT_NE(mxslc::compile_to_string(source, opts).find("name=\"z\""), string::npos);
}

TEST(compile_options_sources, resolves_a_two_level_project_tree)
{
    // The data/groundtruth/directives/include006.mxsl chain, supplied in memory
    // instead of on disk: include006.mxsl -> includes/include1.mxsl ->
    // includes/include2.mxsl. include1.mxsl includes "include2.mxsl", which can
    // only resolve relative to the file that included it: no search directory
    // containing the fixture is set, so there is nothing to fall back on.
    mxslc::CompileOptions opts;
    opts.reduce_graph = false;
    opts.add_source("includes/include1.mxsl",
        "#include \"include2.mxsl\"\n"
        "\n"
        "float foo()\n"
        "{\n"
        "    return bar() + 8.0;\n"
        "}\n"
        "\n"
        "inline float ifoo()\n"
        "{\n"
        "    return bar() + 8.0;\n"
        "}\n");
    opts.add_source("includes/include2.mxsl",
        "float bar()\n"
        "{\n"
        "    return 80.0;\n"
        "}\n"
        "\n"
        "inline float ibar()\n"
        "{\n"
        "    return 80.0;\n"
        "}\n");

    // A root compiled from a string has no directory of its own, so its include
    // is written relative to the conceptual project root.
    const string source =
        "#include \"includes/include1.mxsl\"\n"
        "\n"
        "float a = foo();\n"
        "float b = ifoo();\n"
        "float c = bar();\n"
        "float d = ibar();\n"
        "\n"
        "float z = a + b + c + d;\n";

    const string mtlx = mxslc::compile_to_string(source, opts);

    // bar() / ibar() come only from include2.mxsl, two levels deep.
    EXPECT_NE(mtlx.find("name=\"ND_bar\""), string::npos);
    EXPECT_NE(mtlx.find("name=\"NG_bar\""), string::npos);
    // foo() / ifoo() come from include1.mxsl.
    EXPECT_NE(mtlx.find("name=\"ND_foo\""), string::npos);
    EXPECT_NE(mtlx.find("name=\"NG_foo\""), string::npos);
    EXPECT_NE(mtlx.find("name=\"z\""), string::npos);
}
