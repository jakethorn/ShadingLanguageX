//
// Created by jaket on 30/07/2026.
//

#include "gtest/gtest.h"

#include "utils/data_utils.h"
#include "utils/comp_utils.h"

#include "common.h"
#include "CompileOptions.h"
#include "compile.h"
#include "runtime/Type.h"
#include "runtime/variables/Variable.h"

using namespace mxslc;

TEST(global_tests, test_string_to_path)
{
    CompileOptions opts;
    opts.add_global("tint", 0.3f);
    opts.add_global("albedo_path", "../brick.png");

    const fs::path input_path = get_test_data("globals/globals005.mxsl");
    fs::path expected_path = input_path;
    expected_path.replace_extension(".mtlx");

    const string actual_output = compile_to_string(input_path, opts);

    if constexpr (overwrite_data_files())
        write_file(expected_path, actual_output);

    const string expected_output = read_file(expected_path);
    const bool passed = actual_output == expected_output;

    EXPECT_TRUE(passed);
    if (not passed)
        print_debug_info(input_path, actual_output, expected_output);
}

TEST(global_tests, test_error_on_missing_globals)
{
    CompileOptions opts;
    opts.error_on_missing_globals = true;

    const fs::path input_path = get_test_data("globals/globals006.mxsl");

    EXPECT_THROW(compile_to_string(input_path, opts), mxslc::CompileError);
}

TEST(global_tests, test_no_error_on_missing_globals)
{
    CompileOptions opts;
    opts.error_on_missing_globals = false;

    const fs::path input_path = get_test_data("globals/globals006.mxsl");

    EXPECT_NO_THROW(compile_to_string(input_path, opts));
}

TEST(global_tests, test_error_on_unused_globals_1)
{
    CompileOptions opts;
    opts.error_on_unused_globals = true;
    opts.add_global("i", 1);
    opts.add_global("j", 2);
    opts.add_global("k", 3);
    opts.add_global("x", 4);

    const fs::path input_path = get_test_data("globals/globals006.mxsl");

    EXPECT_THROW(compile_to_string(input_path, opts), mxslc::CompileError);
}

TEST(global_tests, test_error_on_unused_globals_2)
{
    CompileOptions opts;
    opts.error_on_missing_globals = false;
    opts.error_on_unused_globals = true;
    opts.add_global("x", 4);

    const fs::path input_path = get_test_data("globals/globals006.mxsl");

    EXPECT_THROW(compile_to_string(input_path, opts), mxslc::CompileError);
}

TEST(global_tests, test_no_error_on_unused_globals)
{
    CompileOptions opts;
    opts.error_on_unused_globals = false;
    opts.add_global("i", 1);
    opts.add_global("j", 2);
    opts.add_global("k", 3);
    opts.add_global("x", 4);

    const fs::path input_path = get_test_data("globals/globals006.mxsl");

    EXPECT_NO_THROW(compile_to_string(input_path, opts));
}

TEST(global_tests, test_get_globals)
{
    const string source = R"(
global float roughness = 0.25;
global color3 tint;
global {vec3 pos, float size} light;
float unrelated = 1.0;
)";

    const vector<GlobalVariable> globals = get_globals(source);

    ASSERT_EQ(globals.size(), 3);
    EXPECT_EQ(globals[0].name, "roughness");
    EXPECT_EQ(globals[0].type->to_string(), "float");
    EXPECT_FLOAT_EQ(globals[0].value->compile_time_value<float>(), 0.25f);
    EXPECT_EQ(globals[1].name, "tint");
    EXPECT_EQ(globals[1].type->to_string(), "color3");
    EXPECT_EQ(globals[1].value->compile_time_value<mx::Color3>(), mx::Color3(0.0f));
    EXPECT_EQ(globals[2].name, "light");
    EXPECT_EQ(globals[2].value->child_count(), 2);
}

TEST(global_tests, test_get_globals_ignores_global_values)
{
    CompileOptions opts;
    opts.add_global("roughness", 0.9f);

    const vector<GlobalVariable> globals = get_globals("global float roughness = 0.25;"s, opts);

    ASSERT_EQ(globals.size(), 1);
    EXPECT_FLOAT_EQ(globals[0].value->compile_time_value<float>(), 0.25f);
}

TEST(global_tests, test_get_globals_with_runtime_initializer)
{
    const vector<GlobalVariable> globals = get_globals("global float u = texcoord<vec2>().x;"s);

    ASSERT_EQ(globals.size(), 1);
    EXPECT_FALSE(globals[0].value->is_compile_time());
}
