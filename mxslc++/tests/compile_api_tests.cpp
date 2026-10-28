//
// Created by jaket on 28/10/2026.
//

#include "gtest/gtest.h"
#include <string>

#include "CompileOptions.h"
#include "compile.h"
#include "errors/CompileError.h"
#include "utils/data_utils.h"

using std::string;

TEST(compile_api_tests, test_1)
{
    string result = mxslc::compile_to_string("float f = 1 + 1;"s);
}
