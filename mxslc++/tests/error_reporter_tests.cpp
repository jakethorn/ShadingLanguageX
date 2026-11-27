//
// Created by jaket on 23/11/2026.
//

#include "gtest/gtest.h"
#include <filesystem>
#include <string>
#include "Token.h"
#include "errors/CompileError.h"
#include "errors/ErrorReporter.h"
#include "errors/FatalError.h"

namespace fs = std::filesystem;
using std::string;

namespace
{
    mxslc::Token make_token(const size_t line, const fs::path& path = {})
    {
        mxslc::Token token{"x"};
        token.set_line(line);
        if (not path.empty())
            token.set_path(path);
        return token;
    }
}

TEST(error_reporter_tests, error_without_location_is_message_only)
{
    const mxslc::CompileError e{"Invalid statement"};
    EXPECT_STREQ(e.what(), "Invalid statement");
}

TEST(error_reporter_tests, error_location_shows_file_name_and_line)
{
    const mxslc::CompileError e{make_token(3, "/project/shaders/brick.mxsl"), "Invalid statement"};
    EXPECT_STREQ(e.what(), "brick.mxsl, line 3: Invalid statement");
}

TEST(error_reporter_tests, error_location_without_file_shows_line)
{
    const mxslc::CompileError e{make_token(3), "Invalid statement"};
    EXPECT_STREQ(e.what(), "line 3: Invalid statement");
}

TEST(error_reporter_tests, debug_info_only_sets_unknown_locations)
{
    mxslc::CompileError located{make_token(3), "a"};
    located.set_debug_info(make_token(7));
    EXPECT_STREQ(located.what(), "line 3: a");

    mxslc::CompileError unlocated{"b"};
    unlocated.set_debug_info(make_token(0));
    EXPECT_STREQ(unlocated.what(), "b");
    unlocated.set_debug_info(make_token(7));
    EXPECT_STREQ(unlocated.what(), "line 7: b");
}

TEST(error_reporter_tests, reported_errors_are_thrown_together)
{
    mxslc::ErrorReporter reporter{0};
    reporter.report(mxslc::CompileError{make_token(1), "a"});
    reporter.report(mxslc::CompileError{make_token(4), "b"});

    try
    {
        reporter.throw_if_errors();
        FAIL() << "Expected CompileError";
    }
    catch (const mxslc::CompileError& e)
    {
        EXPECT_EQ(e.diagnostics().size(), 2);
        EXPECT_STREQ(e.what(), "line 1: a\nline 4: b");
    }
}

TEST(error_reporter_tests, no_errors_are_not_thrown)
{
    const mxslc::ErrorReporter reporter{0};
    EXPECT_FALSE(reporter.has_errors());
    EXPECT_NO_THROW(reporter.throw_if_errors());
}

TEST(error_reporter_tests, duplicate_errors_are_reported_once)
{
    mxslc::ErrorReporter reporter{0};
    reporter.report(mxslc::CompileError{make_token(2), "a"});
    reporter.report(mxslc::CompileError{make_token(2), "a"});
    reporter.report(mxslc::CompileError{make_token(5), "a"});
    EXPECT_EQ(reporter.diagnostics().size(), 2);
}

TEST(error_reporter_tests, reaching_max_errors_is_fatal)
{
    mxslc::ErrorReporter reporter{2};
    reporter.report(mxslc::CompileError{make_token(1), "a"});

    try
    {
        reporter.report(mxslc::CompileError{make_token(2), "b"});
        FAIL() << "Expected FatalError";
    }
    catch (const mxslc::FatalError& e)
    {
        EXPECT_STREQ(e.what(), "line 1: a\nline 2: b\nToo many errors, stopping compilation");
    }
}

TEST(error_reporter_tests, zero_max_errors_is_unlimited)
{
    mxslc::ErrorReporter reporter{0};
    for (size_t line = 1; line <= 100; ++line)
        reporter.report(mxslc::CompileError{make_token(line), "a"});
    EXPECT_EQ(reporter.diagnostics().size(), 100);
}
