//
// Created by jaket on 12/07/2026.
//

#include <gtest/gtest.h>
#include "primitive/Primitive.h"

using namespace mxslc;

// --- Arithmetic Operators ---

TEST(primitive_tests, arithmetic_addition) 
{
    // Integer addition
    Primitive i1{5};
    Primitive i2{10};
    EXPECT_EQ((i1 + i2).as<int>(), 15);

    Primitive i3{7};
    Primitive i4{true};
    EXPECT_EQ((i3 + i4).as<int>(), 8);

    // Float addition
    Primitive f1{2.5f};
    Primitive f2{1.25f};
    EXPECT_FLOAT_EQ((f1 + f2).as<float>(), 3.75f);

    Primitive f3{20};
    Primitive f4{5.3f};
    EXPECT_FLOAT_EQ((f3 + f4).as<float>(), 25.3f);
    EXPECT_EQ((f3 + f4).cast<int>(), 25);

    Primitive f5{false};
    Primitive f6{5.3f};
    EXPECT_FLOAT_EQ((f5 + f6).as<float>(), 5.3f);

    // String concatenation
    Primitive s1{"Hello, "};
    Primitive s2{"World!"};
    Primitive s_result = s1 + s2;
    EXPECT_TRUE(s_result.is_a<string>());
    EXPECT_EQ(s_result.as<string>(), "Hello, World!");
    
    // Vector addition
    Primitive v1{std::array<float, 2>{1.0f, 2.0f}};
    Primitive v2{std::array<float, 2>{3.0f, 4.0f}};
    Primitive v_result = v1 + v2;
    EXPECT_TRUE(v_result.is_a<mx::Vector2>());
    EXPECT_FLOAT_EQ(v_result.as<mx::Vector2>()[0], 4.0f);
    EXPECT_FLOAT_EQ(v_result.as<mx::Vector2>()[1], 6.0f);
}

TEST(primitive_tests, arithmetic_subtraction) 
{
    EXPECT_EQ((Primitive{10} - Primitive{4}).as<int>(), 6);
    EXPECT_FLOAT_EQ((Primitive{10.5f} - Primitive{2.0f}).as<float>(), 8.5f);

    Primitive v1{std::array<float, 3>{5.0f, 5.0f, 5.0f}};
    Primitive v2{std::array<float, 3>{1.0f, 2.0f, 3.0f}};
    EXPECT_FLOAT_EQ((v1 - v2).as<mx::Vector3>()[0], 4.0f);
    EXPECT_FLOAT_EQ((v1 - v2).as<mx::Vector3>()[1], 3.0f);
    EXPECT_FLOAT_EQ((v1 - v2).as<mx::Vector3>()[2], 2.0f);

    Primitive v3{std::array<float, 3>{5.0f, 6.0f, 4.0f}};
    EXPECT_FLOAT_EQ((v3 - true).as<mx::Vector3>()[0], 4.0f);
    EXPECT_FLOAT_EQ((v3 - true).as<mx::Vector3>()[1], 5.0f);
    EXPECT_FLOAT_EQ((v3 - true).as<mx::Vector3>()[2], 3.0f);
}

TEST(primitive_tests, arithmetic_multiplication) 
{
    // Integer and Float
    EXPECT_EQ((Primitive{5} * Primitive{4}).as<int>(), 20);
    EXPECT_FLOAT_EQ((Primitive{2.5f} * Primitive{2.0f}).as<float>(), 5.0f);
    EXPECT_FLOAT_EQ((Primitive{2.5f} * 2.0f).as<float>(), 5.0f);

    // Vector * Scalar
    Primitive vec{std::array<float, 4>{1.0f, 2.0f, 3.0f, 4.0f}};
    Primitive scalar{2.0f};
    Primitive result = vec * scalar;
    
    EXPECT_TRUE(result.is_a<mx::Vector4>());
    EXPECT_FLOAT_EQ(result.as<mx::Vector4>()[0], 2.0f);
    EXPECT_FLOAT_EQ(result.as<mx::Vector4>()[1], 4.0f);
    EXPECT_FLOAT_EQ(result.as<mx::Vector4>()[2], 6.0f);
    EXPECT_FLOAT_EQ(result.as<mx::Vector4>()[3], 8.0f);

    Primitive vec2{mx::Vector2{8, 9}};
    Primitive scalar2{false};
    Primitive result2 = vec2 * scalar2;

    EXPECT_TRUE(result2.is_a<mx::Vector2>());
    EXPECT_FLOAT_EQ(result2.as<mx::Vector2>()[0], 0.0f);
    EXPECT_FLOAT_EQ(result2.as<mx::Vector2>()[1], 0.0f);
}

TEST(primitive_tests, arithmetic_division_and_path_concat) 
{
    // Standard division
    EXPECT_EQ((Primitive{20} / Primitive{4}).as<int>(), 5);
    EXPECT_FLOAT_EQ((Primitive{10.0f} / Primitive{4.0f}).as<float>(), 2.5f);

    // fs::path operator/ (Path concatenation)
    Primitive base_path{fs::path{"/usr/local"}};
    Primitive sub_path{fs::path{"bin"}};
    Primitive path_result = base_path / sub_path;
    
    EXPECT_TRUE(path_result.is_a<fs::path>());
    EXPECT_EQ(path_result.as<fs::path>(), fs::path{"/usr/local/bin"});
}

// --- Unary Operators ---

TEST(primitive_tests, unary_operators) 
{
    // Negation
    EXPECT_EQ((-Primitive{5}).as<int>(), -5);
    EXPECT_FLOAT_EQ((-Primitive{3.14f}).as<float>(), -3.14f);
    EXPECT_FLOAT_EQ((-Primitive{std::array<float, 2>{1.0f, -2.0f}}).as<mx::Vector2>()[1], 2.0f);

    // Positive
    EXPECT_EQ((+Primitive{-10}).as<int>(), -10);

    // Logical NOT
    EXPECT_FALSE((!Primitive{true}).as<bool>());
    EXPECT_TRUE((!Primitive{false}).as<bool>());
}

// --- Comparison Operators ---

TEST(primitive_tests, comparison_equality) 
{
    // Numeric
    EXPECT_TRUE((Primitive{100} == Primitive{100}).as<bool>());
    EXPECT_TRUE((Primitive{100} != Primitive{99}).as<bool>());

    // Strings
    EXPECT_TRUE((Primitive{"mxslc"} == Primitive{"mxslc"}).as<bool>());
    EXPECT_TRUE((Primitive{"alpha"} != Primitive{"beta"}).as<bool>());

    // fs::path
    EXPECT_TRUE((Primitive{fs::path{"/etc/config"}} == Primitive{fs::path{"/etc/config"}}).as<bool>());
    EXPECT_TRUE((Primitive{fs::path{"/etc/config"}} != Primitive{fs::path{"/var/log"}}).as<bool>());
}

TEST(primitive_tests, comparison_relational) 
{
    Primitive a{5.0f};
    Primitive b{10.0f};

    EXPECT_TRUE((a < b).as<bool>());
    EXPECT_TRUE((a <= b).as<bool>());
    EXPECT_FALSE((a > b).as<bool>());
    EXPECT_FALSE((a >= b).as<bool>());

    EXPECT_TRUE((Primitive{5} <= Primitive{5}).as<bool>());
    EXPECT_TRUE((Primitive{10} < Primitive{15}).as<bool>());

    EXPECT_TRUE((Primitive{4} < Primitive{5.0f}).as<bool>());
    EXPECT_TRUE((Primitive{4.7f} < Primitive{5}).as<bool>());

    EXPECT_TRUE((Primitive{15.0f} >= Primitive{10}).as<bool>());
    EXPECT_TRUE((Primitive{10} <= Primitive{15.3f}).as<bool>());

    EXPECT_TRUE((Primitive{10} < Primitive{10.3f}).as<bool>());
    EXPECT_TRUE((Primitive{10} < Primitive{10.001f}).as<bool>());
    EXPECT_TRUE((Primitive{10} > Primitive{9.999f}).as<bool>());
    EXPECT_TRUE((Primitive{9.7f} < Primitive{10.3f}).as<bool>());
}

// --- Logical Operators ---

TEST(primitive_tests, logical_operators) 
{
    Primitive t{true};
    Primitive f{false};

    EXPECT_TRUE((t and t).as<bool>());
    EXPECT_FALSE((t and f).as<bool>());
    EXPECT_FALSE((f and t).as<bool>());
    EXPECT_FALSE((f and f).as<bool>());

    EXPECT_TRUE((t or t).as<bool>());
    EXPECT_TRUE((t or f).as<bool>());
    EXPECT_TRUE((f or t).as<bool>());
    EXPECT_FALSE((f or f).as<bool>());
    
    EXPECT_TRUE((t xor f).as<bool>());
    EXPECT_TRUE((f xor t).as<bool>());
    EXPECT_FALSE((t xor t).as<bool>());
    EXPECT_FALSE((f xor f).as<bool>());

    EXPECT_TRUE((t and 1).as<bool>());
    EXPECT_FALSE((t and 0).as<bool>());
}

// --- Subscript Operator ---

TEST(primitive_tests, subscript_access) 
{
    Primitive vec{std::array<float, 3>{7.0f, 8.0f, 9.0f}};
    
    // Extracted using size_t
    EXPECT_FLOAT_EQ(vec[1].as<float>(), 8.0f);
    
    // Extracted using Primitive{int}
    EXPECT_FLOAT_EQ(vec[Primitive{2}].as<float>(), 9.0f);
}