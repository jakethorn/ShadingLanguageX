//
// Created by jaket on 03/01/2026.
//

#include "gtest/gtest.h"
#include "parse_args.h"
#include "Span.h"
#include "utils/data_utils.h"

using std::string;
using std::vector;
using namespace std::string_literals;
namespace fs = std::filesystem;

#define EXECUTABLE "mxslc"s

TEST(parse_args_tests, test_input_filepath)
{
    const fs::path input_filepath = get_test_data("parse_args_tests/001.mxsl"s);

    const vector argv{ EXECUTABLE, input_filepath.string() };
    const mxslc::Args args = mxslc::parse_args(argv);

    ASSERT_TRUE(args.is_valid);
    ASSERT_EQ(args.input_file, input_filepath);
    ASSERT_FALSE(args.output_file);
    ASSERT_FALSE(args.version);
}

TEST(parse_args_tests, test_relative_input_filepath)
{
    const fs::path input_filepath = "tests/data/parse_args_tests/001.mxsl"s;

    const vector argv{ EXECUTABLE, input_filepath.string() };
    const mxslc::Args args = mxslc::parse_args(argv);

    ASSERT_TRUE(args.is_valid);
    ASSERT_EQ(args.input_file, get_test_data("parse_args_tests/001.mxsl"s));
    ASSERT_TRUE(args.input_file.is_absolute());
    ASSERT_FALSE(args.output_file);
    ASSERT_FALSE(args.version);
}

TEST(parse_args_tests, test_no_input_filepath)
{
    const vector argv{ EXECUTABLE };
    const mxslc::Args args = mxslc::parse_args(argv);

    ASSERT_FALSE(args.is_valid);
}

TEST(parse_args_tests, test_bad_input_filepath)
{
    const fs::path input_filepath = get_test_data("parse_args_tests/not_a_file.mxsl"s);

    const vector argv{ EXECUTABLE, input_filepath.string() };
    const mxslc::Args args = mxslc::parse_args(argv);

    ASSERT_FALSE(args.is_valid);
}

TEST(parse_args_tests, test_options_001)
{
    const fs::path input_filepath = get_test_data("parse_args_tests/001.mxsl"s);
    const fs::path output_filepath = get_test_data("parse_args_tests/001.mtlx"s);

    const vector argv{ EXECUTABLE, input_filepath.string(), "-o"s, output_filepath.string() };
    const mxslc::Args args = mxslc::parse_args(argv);

    ASSERT_TRUE(args.is_valid);
    ASSERT_EQ(args.input_file, input_filepath);
    ASSERT_EQ(args.output_file, output_filepath);
    ASSERT_FALSE(args.version);
}

TEST(parse_args_tests, test_options_002)
{
    const fs::path input_filepath = get_test_data("parse_args_tests/001.mxsl"s);
    const fs::path output_filepath = get_test_data("parse_args_tests/001.mtlx"s);

    const vector argv{ EXECUTABLE, input_filepath.string(), "--output-file"s, output_filepath.string(), "-v"s, "1.39.4"s };
    const mxslc::Args args = mxslc::parse_args(argv);

    ASSERT_TRUE(args.is_valid);
    ASSERT_EQ(args.input_file, input_filepath);
    ASSERT_EQ(args.output_file, output_filepath);
    ASSERT_EQ(args.version, "1.39.4"s);
}

TEST(parse_args_tests, test_options_003)
{
    const fs::path input_filepath = get_test_data("parse_args_tests/001.mxsl"s);

    const vector argv{ EXECUTABLE, input_filepath.string(), "--version"s, "1.39.4"s };
    const mxslc::Args args = mxslc::parse_args(argv);

    ASSERT_TRUE(args.is_valid);
    ASSERT_EQ(args.input_file, input_filepath);
    ASSERT_FALSE(args.output_file);
    ASSERT_EQ(args.version, "1.39.4"s);
}

TEST(parse_args_tests, test_bad_options_001)
{
    const fs::path input_filepath = get_test_data("parse_args_tests/001.mxsl"s);

    const vector argv{ EXECUTABLE, input_filepath.string(), "-z"s };
    const mxslc::Args args = mxslc::parse_args(argv);

    ASSERT_FALSE(args.is_valid);
}

TEST(parse_args_tests, test_bad_options_002)
{
    const fs::path input_filepath = get_test_data("parse_args_tests/001.mxsl"s);

    const vector argv{ EXECUTABLE, input_filepath.string(), "-o"s };
    const mxslc::Args args = mxslc::parse_args(argv);

    ASSERT_FALSE(args.is_valid);
}

TEST(parse_args_tests, test_bad_options_003)
{
    const fs::path input_filepath = get_test_data("parse_args_tests/001.mxsl"s);
    const fs::path output_filepath = get_test_data("parse_args_tests/001.mtlx"s);

    const vector argv{ EXECUTABLE, input_filepath.string(), output_filepath.string(), "-v"s, "1.39.4"s };
    const mxslc::Args args = mxslc::parse_args(argv);

    ASSERT_FALSE(args.is_valid);
}

TEST(parse_args_tests, test_bad_version)
{
    const fs::path input_filepath = get_test_data("parse_args_tests/001.mxsl"s);

    const vector argv{ EXECUTABLE, input_filepath.string(), "-v"s, "1.38.10"s };
    const mxslc::Args args = mxslc::parse_args(argv);

    ASSERT_FALSE(args.is_valid);
}

TEST(parse_args_tests, test_response_file_001)
{
    const fs::path response_filepath = get_test_data("parse_args_tests/001.rsp"s);

    const vector argv{ EXECUTABLE, "@"s + response_filepath.string() };
    const mxslc::Args args = mxslc::parse_args(argv);

    ASSERT_TRUE(args.is_valid);
    ASSERT_EQ(args.input_file, get_test_data("parse_args_tests/001.mxsl"s));
    ASSERT_TRUE(args.input_file.is_absolute());
    ASSERT_FALSE(args.output_file);
    ASSERT_FALSE(args.version);
}

TEST(parse_args_tests, test_response_file_002)
{
    const fs::path response_filepath = get_test_data("parse_args_tests/002.rsp"s);

    const vector argv{ EXECUTABLE, "@"s + response_filepath.string() };
    const mxslc::Args args = mxslc::parse_args(argv);

    ASSERT_TRUE(args.is_valid);
    ASSERT_EQ(args.input_file, get_test_data("parse_args_tests/001.mxsl"s));
    ASSERT_TRUE(args.input_file.is_absolute());
    ASSERT_EQ(args.output_file, get_test_data("parse_args_tests/001.mtlx"s));
    ASSERT_TRUE(args.output_file->is_absolute());
    ASSERT_EQ(args.version, "1.39.4"s);
}

TEST(parse_args_tests, test_response_file_003)
{
    const fs::path response_filepath = get_test_data("parse_args_tests/003.rsp"s);

    const vector argv{ EXECUTABLE, "@"s + response_filepath.string() };
    const mxslc::Args args = mxslc::parse_args(argv);

    ASSERT_TRUE(args.is_valid);
    ASSERT_EQ(args.input_file, get_test_data("parse_args_tests/001.mxsl"s));
    ASSERT_TRUE(args.input_file.is_absolute());
    ASSERT_EQ(args.output_file, get_test_data("parse_args_tests/001.mtlx"s));
    ASSERT_TRUE(args.output_file->is_absolute());
    ASSERT_EQ(args.version, "1.39.4"s);
}

TEST(parse_args_tests, test_response_file_004)
{
    const fs::path response_filepath = get_test_data("parse_args_tests/001.rsp"s);

    const vector argv{ EXECUTABLE, "@"s + response_filepath.string(), "--ignored-option"s };
    const mxslc::Args args = mxslc::parse_args(argv);

    ASSERT_TRUE(args.is_valid);
    ASSERT_EQ(args.input_file, get_test_data("parse_args_tests/001.mxsl"s));
    ASSERT_TRUE(args.input_file.is_absolute());
    ASSERT_FALSE(args.output_file);
    ASSERT_FALSE(args.version);
}

TEST(parse_args_tests, test_bad_response_file)
{
    const fs::path response_filepath = get_test_data("parse_args_tests/004.rsp"s);

    const vector argv{ EXECUTABLE, "@"s + response_filepath.string() };
    const mxslc::Args args = mxslc::parse_args(argv);

    ASSERT_FALSE(args.is_valid);
}
