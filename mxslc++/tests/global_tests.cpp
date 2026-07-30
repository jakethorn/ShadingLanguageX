//
// Created by jaket on 30/07/2026.
//

#include "gtest/gtest.h"

#include "utils/data_utils.h"
#include "utils/comp_utils.h"

#include "common.h"
#include "CompileOptions.h"
#include "compile.h"

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
