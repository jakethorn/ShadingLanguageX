//
// Created by jaket on 15/07/2026.
//

#include <gtest/gtest.h>
#include "Primitive.h"

using namespace mxslc;

// --- Arithmetic Operators ---

TEST(primitive_tests, bool_to_string)
{
    ASSERT_EQ(Primitive{true}.to_string(), "true");
    ASSERT_EQ(Primitive{false}.to_string(), "false");
}

TEST(primitive_tests, int_to_string)
{
    ASSERT_EQ(Primitive{0}.to_string(), "0");
    ASSERT_EQ(Primitive{1}.to_string(), "1");
    ASSERT_EQ(Primitive{7000}.to_string(), "7000");
}

TEST(primitive_tests, string_to_string)
{
    ASSERT_EQ(Primitive{""}.to_string(), "\"\"");
    ASSERT_EQ(Primitive{"hello"}.to_string(), "\"hello\"");
}

TEST(primitive_tests, filename_to_string)
{
    ASSERT_EQ(Primitive{fs::path{"image.png"}}.to_string(), "\"image.png\"");
}

TEST(primitive_tests, vec2_to_string)
{
    ASSERT_EQ(Primitive{mx::Vector2{}}.to_string(), "vec2{0, 0}");
}

TEST(primitive_tests, vec3_to_string)
{
    ASSERT_EQ(Primitive{mx::Vector3{}}.to_string(), "vec3{0, 0, 0}");
}

TEST(primitive_tests, vec4_to_string)
{
    ASSERT_EQ(Primitive{mx::Vector4{}}.to_string(), "vec4{0, 0, 0, 0}");
}
