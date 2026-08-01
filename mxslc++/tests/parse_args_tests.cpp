//
// Created by jaket on 03/01/2026.
//

#include "gtest/gtest.h"
#include "utils/parse_cli_args.h"
#include "runtime/Type.h"
#include "runtime/Variable.h"
#include "utils/data_utils.h"

using std::string;
using std::vector;
using namespace std::string_literals;
namespace fs = std::filesystem;

#define EXECUTABLE "mxslc"s
#define DEFAULT_MTLX_VERSION "1.39.5"s

TEST(parse_args_tests, test_input_filepath)
{
    const fs::path input_filepath = get_test_data("parse_args_tests/001.mxsl"s);

    const vector argv{ EXECUTABLE, input_filepath.string() };
    const mxslc::CommandLineArgs args = mxslc::parse_cli_args(argv);

    ASSERT_TRUE(args.is_valid);
    ASSERT_EQ(args.input_file, input_filepath);
    ASSERT_FALSE(args.options.output_file);
    ASSERT_EQ(args.options.version, DEFAULT_MTLX_VERSION);
}

TEST(parse_args_tests, test_relative_input_filepath)
{
    const fs::path input_filepath = "tests/data/parse_args_tests/001.mxsl"s;

    const vector argv{ EXECUTABLE, input_filepath.string() };
    const mxslc::CommandLineArgs args = mxslc::parse_cli_args(argv);

    ASSERT_TRUE(args.is_valid);
    ASSERT_EQ(args.input_file, get_test_data("parse_args_tests/001.mxsl"s));
    ASSERT_TRUE(args.input_file.is_absolute());
    ASSERT_FALSE(args.options.output_file);
    ASSERT_EQ(args.options.version, DEFAULT_MTLX_VERSION);
}

TEST(parse_args_tests, test_no_input_filepath)
{
    const vector argv{ EXECUTABLE };
    const mxslc::CommandLineArgs args = mxslc::parse_cli_args(argv);

    ASSERT_FALSE(args.is_valid);
}

TEST(parse_args_tests, test_bad_input_filepath)
{
    const fs::path input_filepath = get_test_data("parse_args_tests/not_a_file.mxsl"s);

    const vector argv{ EXECUTABLE, input_filepath.string() };
    const mxslc::CommandLineArgs args = mxslc::parse_cli_args(argv);

    ASSERT_FALSE(args.is_valid);
}

TEST(parse_args_tests, test_options_001)
{
    const fs::path input_filepath = get_test_data("parse_args_tests/001.mxsl"s);
    const fs::path output_filepath = get_test_data("parse_args_tests/001.mtlx"s);

    const vector argv{ EXECUTABLE, input_filepath.string(), "-o"s, output_filepath.string() };
    const mxslc::CommandLineArgs args = mxslc::parse_cli_args(argv);

    ASSERT_TRUE(args.is_valid);
    ASSERT_EQ(args.input_file, input_filepath);
    ASSERT_EQ(args.options.output_file, output_filepath);
    ASSERT_EQ(args.options.version, DEFAULT_MTLX_VERSION);
    ASSERT_TRUE(args.options.reduce_graph);
    ASSERT_TRUE(args.options.error_on_missing_globals);
    ASSERT_TRUE(args.options.error_on_unused_globals);
}

TEST(parse_args_tests, test_options_002)
{
    const fs::path input_filepath = get_test_data("parse_args_tests/001.mxsl"s);
    const fs::path output_filepath = get_test_data("parse_args_tests/001.mtlx"s);

    const vector argv{ EXECUTABLE, input_filepath.string(), "--output-file"s, output_filepath.string(), "-v"s, "1.38.10"s };
    const mxslc::CommandLineArgs args = mxslc::parse_cli_args(argv);

    ASSERT_TRUE(args.is_valid);
    ASSERT_EQ(args.input_file, input_filepath);
    ASSERT_EQ(args.options.output_file, output_filepath);
    ASSERT_EQ(args.options.version, "1.38.10"s);
    ASSERT_TRUE(args.options.reduce_graph);
    ASSERT_TRUE(args.options.error_on_missing_globals);
    ASSERT_TRUE(args.options.error_on_unused_globals);
}

TEST(parse_args_tests, test_options_003)
{
    const fs::path input_filepath = get_test_data("parse_args_tests/001.mxsl"s);

    const vector argv{ EXECUTABLE, input_filepath.string(), "--version"s, "1.38.10"s };
    const mxslc::CommandLineArgs args = mxslc::parse_cli_args(argv);

    ASSERT_TRUE(args.is_valid);
    ASSERT_EQ(args.input_file, input_filepath);
    ASSERT_FALSE(args.options.output_file);
    ASSERT_EQ(args.options.version, "1.38.10"s);
    ASSERT_TRUE(args.options.reduce_graph);
    ASSERT_TRUE(args.options.error_on_missing_globals);
    ASSERT_TRUE(args.options.error_on_unused_globals);
}

TEST(parse_args_tests, test_options_004)
{
    const fs::path input_filepath = get_test_data("parse_args_tests/001.mxsl"s);

    const vector argv{ EXECUTABLE, input_filepath.string(), "--no-reduce-graph"s };
    const mxslc::CommandLineArgs args = mxslc::parse_cli_args(argv);

    ASSERT_TRUE(args.is_valid);
    ASSERT_EQ(args.input_file, input_filepath);
    ASSERT_FALSE(args.options.output_file);
    ASSERT_EQ(args.options.version, DEFAULT_MTLX_VERSION);
    ASSERT_FALSE(args.options.reduce_graph);
    ASSERT_TRUE(args.options.error_on_missing_globals);
    ASSERT_TRUE(args.options.error_on_unused_globals);
}

TEST(parse_args_tests, test_bad_options_001)
{
    const fs::path input_filepath = get_test_data("parse_args_tests/001.mxsl"s);

    const vector argv{ EXECUTABLE, input_filepath.string(), "-z"s };
    const mxslc::CommandLineArgs args = mxslc::parse_cli_args(argv);

    ASSERT_FALSE(args.is_valid);
}

TEST(parse_args_tests, test_bad_options_002)
{
    const fs::path input_filepath = get_test_data("parse_args_tests/001.mxsl"s);

    const vector argv{ EXECUTABLE, input_filepath.string(), "-o"s };
    const mxslc::CommandLineArgs args = mxslc::parse_cli_args(argv);

    ASSERT_FALSE(args.is_valid);
}

TEST(parse_args_tests, test_bad_options_003)
{
    const fs::path input_filepath = get_test_data("parse_args_tests/001.mxsl"s);
    const fs::path output_filepath = get_test_data("parse_args_tests/001.mtlx"s);

    const vector argv{ EXECUTABLE, input_filepath.string(), output_filepath.string(), "-v"s, "1.39.4"s };
    const mxslc::CommandLineArgs args = mxslc::parse_cli_args(argv);

    ASSERT_FALSE(args.is_valid);
}

TEST(parse_args_tests, test_response_file_001)
{
    const fs::path response_filepath = get_test_data("parse_args_tests/001.rsp"s);

    const vector argv{ EXECUTABLE, "@"s + response_filepath.string() };
    const mxslc::CommandLineArgs args = mxslc::parse_cli_args(argv);

    ASSERT_TRUE(args.is_valid);
    ASSERT_EQ(args.input_file, get_test_data("parse_args_tests/001.mxsl"s));
    ASSERT_TRUE(args.input_file.is_absolute());
    ASSERT_FALSE(args.options.output_file);
    ASSERT_EQ(args.options.version, DEFAULT_MTLX_VERSION);
}

TEST(parse_args_tests, test_response_file_002)
{
    const fs::path response_filepath = get_test_data("parse_args_tests/002.rsp"s);

    const vector argv{ EXECUTABLE, "@"s + response_filepath.string() };
    const mxslc::CommandLineArgs args = mxslc::parse_cli_args(argv);

    ASSERT_TRUE(args.is_valid);
    ASSERT_EQ(args.input_file, get_test_data("parse_args_tests/001.mxsl"s));
    ASSERT_TRUE(args.input_file.is_absolute());
    ASSERT_EQ(args.options.output_file, get_test_data("parse_args_tests/001.mtlx"s));
    ASSERT_TRUE(args.options.output_file->is_absolute());
    ASSERT_EQ(args.options.version, "1.39.4"s);
}

TEST(parse_args_tests, test_response_file_003)
{
    const fs::path response_filepath = get_test_data("parse_args_tests/003.rsp"s);

    const vector argv{ EXECUTABLE, "@"s + response_filepath.string() };
    const mxslc::CommandLineArgs args = mxslc::parse_cli_args(argv);

    ASSERT_TRUE(args.is_valid);
    ASSERT_EQ(args.input_file, get_test_data("parse_args_tests/001.mxsl"s));
    ASSERT_TRUE(args.input_file.is_absolute());
    ASSERT_EQ(args.options.output_file, get_test_data("parse_args_tests/001.mtlx"s));
    ASSERT_TRUE(args.options.output_file->is_absolute());
    ASSERT_EQ(args.options.version, "1.39.4"s);
}

TEST(parse_args_tests, test_response_file_004)
{
    const fs::path response_filepath = get_test_data("parse_args_tests/001.rsp"s);

    const vector argv{ EXECUTABLE, "@"s + response_filepath.string(), "--ignored-option"s };
    const mxslc::CommandLineArgs args = mxslc::parse_cli_args(argv);

    ASSERT_TRUE(args.is_valid);
    ASSERT_EQ(args.input_file, get_test_data("parse_args_tests/001.mxsl"s));
    ASSERT_TRUE(args.input_file.is_absolute());
    ASSERT_FALSE(args.options.output_file);
    ASSERT_EQ(args.options.version, DEFAULT_MTLX_VERSION);
}

TEST(parse_args_tests, test_bad_response_file)
{
    const fs::path response_filepath = get_test_data("parse_args_tests/004.rsp"s);

    const vector argv{ EXECUTABLE, "@"s + response_filepath.string() };
    const mxslc::CommandLineArgs args = mxslc::parse_cli_args(argv);

    ASSERT_FALSE(args.is_valid);
}

TEST(parse_args_tests, test_options_005)
{
    const fs::path input_filepath = get_test_data("parse_args_tests/001.mxsl"s);

    const vector argv{ EXECUTABLE, input_filepath.string(), "--func"s, "main"s, "--args"s, "3.4"s };
    const mxslc::CommandLineArgs args = mxslc::parse_cli_args(argv);

    ASSERT_TRUE(args.is_valid);
    ASSERT_EQ(args.options.func_name, "main"s);
    const auto& func_args = args.options.entry_function_arguments();
    ASSERT_EQ(func_args.size(), 1);
    ASSERT_TRUE(func_args[0]->is_basic<float>());
    ASSERT_FLOAT_EQ(func_args[0]->basic<float>(), 3.4);
}

TEST(parse_args_tests, test_response_file_005)
{
    const fs::path response_filepath = get_test_data("parse_args_tests/005.rsp"s);

    const vector argv{ EXECUTABLE, "@"s + response_filepath.string() };
    const mxslc::CommandLineArgs args = mxslc::parse_cli_args(argv);

    ASSERT_TRUE(args.is_valid);

    const auto& func_args = args.options.entry_function_arguments();
    ASSERT_EQ(func_args.size(), 3);

    ASSERT_TRUE(func_args[0]->is_basic<string>());
    ASSERT_EQ(func_args[0]->basic<string>(), "hello world");

    ASSERT_TRUE(func_args[1]->is_basic<float>());
    ASSERT_FLOAT_EQ(func_args[1]->basic<float>(), -3.4);

    ASSERT_TRUE(func_args[2]->is_basic<int>());
    ASSERT_EQ(func_args[2]->basic<int>(), 123);
}

TEST(parse_args_tests, test_response_file_006)
{
    const fs::path response_filepath = get_test_data("parse_args_tests/006.rsp"s);

    const vector argv{ EXECUTABLE, "@"s + response_filepath.string() };
    const mxslc::CommandLineArgs args = mxslc::parse_cli_args(argv);

    ASSERT_TRUE(args.is_valid);

    const auto& func_args = args.options.entry_function_arguments();
    ASSERT_EQ(func_args.size(), 3);

    ASSERT_TRUE(func_args[0]->is_basic<string>());
    ASSERT_EQ(func_args[0]->basic<string>(), "hello world");

    ASSERT_TRUE(func_args[1]->is_basic<float>());
    ASSERT_FLOAT_EQ(func_args[1]->basic<float>(), -3.4);

    ASSERT_TRUE(func_args[2]->is_basic<int>());
    ASSERT_EQ(func_args[2]->basic<int>(), 123);
}

TEST(parse_args_tests, test_global_options)
{
    const fs::path input_filepath = get_test_data("parse_args_tests/001.mxsl"s);

    const vector<string> argv{ EXECUTABLE, input_filepath.string(), "-g", "x", "1.0", "y", "\"hello\"", "--globals", "z", "2.0", "w", "\"world\"", "-g", "u", "10" };
    const mxslc::CommandLineArgs args = mxslc::parse_cli_args(argv);

    ASSERT_TRUE(args.is_valid);

    ASSERT_TRUE(args.options.error_on_missing_globals);
    ASSERT_TRUE(args.options.error_on_unused_globals);

    ASSERT_EQ(args.options.globals().size(), 5);

    const mxslc::VarPtr& x = args.options.get_global("x");
    ASSERT_TRUE(x->type()->is<float>());
    ASSERT_FLOAT_EQ(x->basic<float>(), 1.0);

    const mxslc::VarPtr& y = args.options.get_global("y");
    ASSERT_TRUE(y->type()->is<string>());
    ASSERT_EQ(y->basic<string>(), "hello");

    const mxslc::VarPtr& z = args.options.get_global("z");
    ASSERT_TRUE(z->type()->is<float>());
    ASSERT_FLOAT_EQ(z->basic<float>(), 2.0);

    const mxslc::VarPtr& w = args.options.get_global("w");
    ASSERT_TRUE(w->type()->is<string>());
    ASSERT_EQ(w->basic<string>(), "world");

    const mxslc::VarPtr& u = args.options.get_global("u");
    ASSERT_TRUE(u->type()->is<int>());
    ASSERT_EQ(u->basic<int>(), 10);
}

TEST(parse_args_tests, test_global_error_options)
{
    const fs::path input_filepath = get_test_data("parse_args_tests/001.mxsl"s);

    const vector<string> argv{ EXECUTABLE, input_filepath.string(), "--missing-globals-ok", "--unused-globals-ok" };
    const mxslc::CommandLineArgs args = mxslc::parse_cli_args(argv);

    ASSERT_TRUE(args.is_valid);
    ASSERT_FALSE(args.options.error_on_missing_globals);
    ASSERT_FALSE(args.options.error_on_unused_globals);
}

TEST(parse_args_tests, test_bad_global_options_1)
{
    const fs::path input_filepath = get_test_data("parse_args_tests/001.mxsl"s);

    const vector<string> argv{ EXECUTABLE, input_filepath.string(), "-g", "x", "-f", "main" };
    const mxslc::CommandLineArgs args = mxslc::parse_cli_args(argv);

    ASSERT_FALSE(args.is_valid);
}

TEST(parse_args_tests, test_bad_global_options_2)
{
    const fs::path input_filepath = get_test_data("parse_args_tests/001.mxsl"s);

    const vector<string> argv{ EXECUTABLE, input_filepath.string(), "-g", "x", "_" };
    const mxslc::CommandLineArgs args = mxslc::parse_cli_args(argv);

    ASSERT_FALSE(args.is_valid);
}
