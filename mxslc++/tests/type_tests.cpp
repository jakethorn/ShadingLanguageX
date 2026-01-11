//
// Created by jaket on 09/01/2026.
//

#include "gtest/gtest.h"
#include "runtime/Type.h"
#include "Token.h"

TEST(type_tests, empty_constructor)
{
    const Type t;
    ASSERT_EQ(t.str(), "__not_a_type__"s);
    ASSERT_FALSE(t.is_complex());
}

TEST(type_tests, string_constructor)
{
    const Type t{"float"s};
    ASSERT_EQ(t.str(), "float"s);
    ASSERT_FALSE(t.is_complex());
}

TEST(type_tests, token_constructor)
{
    const Token token{"boolean"s};
    const Type type{token};
    ASSERT_EQ(type.str(), "boolean"s);
    ASSERT_FALSE(type.is_complex());
}

TEST(type_tests, basic_t_constructor)
{
    const Type b_type{true};
    ASSERT_EQ(b_type.str(), "boolean"s);
    ASSERT_FALSE(b_type.is_complex());

    const Type i_type{100};
    ASSERT_EQ(i_type.str(), "integer"s);
    ASSERT_FALSE(i_type.is_complex());

    const Type f_type{3.14f};
    ASSERT_EQ(f_type.str(), "float"s);
    ASSERT_FALSE(f_type.is_complex());

    const Type v_type{basic_t{"world"s}};
    ASSERT_EQ(v_type.str(), "string"s);
    ASSERT_FALSE(v_type.is_complex());
}

TEST(type_tests, vec_string_constructor_001)
{
    const vector types{"vector2"s, "color4"s, "matrix33"s};
    const Type t{types};
    ASSERT_EQ(t.str(), "<vector2, color4, matrix33>"s);
    ASSERT_TRUE(t.is_complex());
}

TEST(type_tests, vec_string_constructor_002)
{
    const vector types{"vector2"s};
    const Type t{types};
    ASSERT_EQ(t.str(), "vector2"s);
    ASSERT_FALSE(t.is_complex());
}

TEST(type_tests, vec_type_constructor)
{
    vector<Type> tuple1;
    tuple1.emplace_back("boolean"s);
    tuple1.emplace_back("string"s);
    vector<Type> tuple2;
    tuple2.emplace_back(tuple1);
    tuple2.emplace_back(33.3f);
    const Type t{std::move(tuple2)};
    ASSERT_EQ(t.str(), "<<boolean, string>, float>"s);
    ASSERT_TRUE(t.is_complex());
}

TEST(type_tests, instantiate_template_type_001)
{
    vector<Type> tuple1;
    tuple1.emplace_back("boolean"s);
    tuple1.emplace_back("T"s);
    vector<Type> tuple2;
    tuple2.emplace_back(tuple1);
    tuple2.emplace_back("T"s);
    const Type t1{std::move(tuple2)};
    ASSERT_EQ(t1.str(), "<<boolean, T>, T>"s);
    const Type t2 = t1.instantiate_template_types("vector2"s);
    ASSERT_EQ(t2.str(), "<<boolean, vector2>, vector2>"s);
}

TEST(type_tests, instantiate_template_type_002)
{
    const Type t1{"T"s};
    ASSERT_EQ(t1.str(), "T"s);
    const Type t2 = t1.instantiate_template_types("filename"s);
    ASSERT_EQ(t2.str(), "filename"s);
}
