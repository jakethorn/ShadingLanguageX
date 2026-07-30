//
// Created by jaket on 12/07/2026.
//

#include "gtest/gtest.h"
#include "Primitive.h"
#include "utils/primitive_utils.h"
#include "runtime/Type.h"

using namespace mxslc;

TEST(primitive_tests, test_combine)
{
    const Primitive a = primitive_utils::combine({0, 0}, Type::Vec2);
    ASSERT_TRUE(a.is_a<mx::Vector2>());

    const Primitive b = primitive_utils::combine({a, 0}, Type::Color3);
    ASSERT_TRUE(b.is_a<mx::Color3>());

    const Primitive c = primitive_utils::combine({0, b}, Type::Vec4);
    ASSERT_TRUE(c.is_a<mx::Vector4>());

    constexpr std::array<float, 2> z{0, 0};
    const Primitive d = primitive_utils::combine({z, z}, Type::Vec4);
    ASSERT_TRUE(d.is_a<mx::Vector4>());

    const Primitive e = primitive_utils::combine({0, mx::Vector2{}, 0}, Type::Vec4);
    ASSERT_TRUE(e.is_a<mx::Vector4>());

    const Primitive f = primitive_utils::combine({0, true, false, 7.0f}, Type::Color4);
    ASSERT_TRUE(f.is_a<mx::Color4>());
}

TEST(primitive_tests, test_combine_fails)
{
    ASSERT_THROW({
        const Primitive a = primitive_utils::combine({1.0f, "2.0f"}, Type::Vec2);
    }, CompileError);

    ASSERT_THROW({
        const Primitive a = primitive_utils::combine({1.0f, mx::Vector4{}}, Type::Vec2);
    }, CompileError);

    ASSERT_THROW({
        const Primitive a = primitive_utils::combine({0, 0, 0}, Type::Vec4);
    }, CompileError);

    ASSERT_THROW({
        const Primitive a = primitive_utils::combine({0, 0, 0, 0, 0}, Type::Vec4);
    }, CompileError);
}

TEST(primitive_tests, test_separate)
{
    const vector<Primitive> a = primitive_utils::separate(mx::Vector2{1.0f, 2.0f});
    ASSERT_EQ(a.size(), 2);
    ASSERT_TRUE(a[0].is_a<float>());
    ASSERT_TRUE(a[1].is_a<float>());
    ASSERT_FLOAT_EQ(a[0].as<float>(), 1.0f);
    ASSERT_FLOAT_EQ(a[1].as<float>(), 2.0f);

    const vector<Primitive> b = primitive_utils::separate(mx::Color4{1.0f, 2.0f, 3.0f, 4.0f});
    ASSERT_EQ(b.size(), 4);
    ASSERT_TRUE(b[0].is_a<float>());
    ASSERT_TRUE(b[1].is_a<float>());
    ASSERT_TRUE(b[2].is_a<float>());
    ASSERT_TRUE(b[3].is_a<float>());
    ASSERT_FLOAT_EQ(b[0].as<float>(), 1.0f);
    ASSERT_FLOAT_EQ(b[1].as<float>(), 2.0f);
    ASSERT_FLOAT_EQ(b[2].as<float>(), 3.0f);
    ASSERT_FLOAT_EQ(b[3].as<float>(), 4.0f);
}

TEST(primitive_tests, test_separate_fails)
{
    ASSERT_THROW({
        const vector<Primitive> a = primitive_utils::separate(mx::Matrix33{});
    }, CompileError);

    ASSERT_THROW({
        const vector<Primitive> a = primitive_utils::separate(mx::Matrix44{});
    }, CompileError);

    ASSERT_THROW({
        const vector<Primitive> a = primitive_utils::separate("hello");
    }, CompileError);

    ASSERT_THROW({
        const vector<Primitive> a = primitive_utils::separate(1.0f);
    }, CompileError);

    ASSERT_THROW({
        const vector<Primitive> a = primitive_utils::separate(1);
    }, CompileError);

    ASSERT_THROW({
        const vector<Primitive> a = primitive_utils::separate(true);
    }, CompileError);
}

TEST(primitive_tests, test_convert)
{
#define ASSERT_IS_CONVERTABLE_TO_SELF(type) ASSERT_TRUE(primitive_utils::convert(type{}, Type::of<type>()).is_a<type>());
    FOR_EACH_PRIMITIVE_TYPE(ASSERT_IS_CONVERTABLE_TO_SELF, )
#undef ASSERT_IS_CASTABLE_TO_SELF

    const Primitive a = primitive_utils::convert(1.0f, Type::Vec3);
    ASSERT_TRUE(a.is_a<mx::Vector3>());
    ASSERT_FLOAT_EQ(a[0].as<float>(), 1.0f);
    ASSERT_FLOAT_EQ(a[1].as<float>(), 1.0f);
    ASSERT_FLOAT_EQ(a[2].as<float>(), 1.0f);

    const Primitive b = primitive_utils::convert(true, Type::Vec2);
    ASSERT_TRUE(b.is_a<mx::Vector2>());
    ASSERT_FLOAT_EQ(b[0].as<float>(), 1.0f);
    ASSERT_FLOAT_EQ(b[1].as<float>(), 1.0f);

    const Primitive c = primitive_utils::convert(0, Type::Bool);
    ASSERT_TRUE(c.is_a<bool>());
    ASSERT_FALSE(c.as<bool>());

    const Primitive d = primitive_utils::convert(mx::Vector4{}, Type::Vec3);
    ASSERT_TRUE(d.is_a<mx::Vector3>());

    const Primitive e = primitive_utils::convert(mx::Vector3{}, Type::Color3);
    ASSERT_TRUE(e.is_a<mx::Color3>());

    const Primitive f = primitive_utils::convert(mx::Color4{}, Type::Vec2);
    ASSERT_TRUE(f.is_a<mx::Vector2>());

    const Primitive g = primitive_utils::convert("hello", Type::Filename);
    ASSERT_TRUE(g.is_a<fs::path>());

    const Primitive h = primitive_utils::convert(fs::path{}, Type::String);
    ASSERT_TRUE(h.is_a<string>());
}

TEST(primitive_tests, test_convert_fails)
{
    ASSERT_THROW({
        const Primitive a = primitive_utils::convert("hello", Type::Int);
    }, CompileError);

    ASSERT_THROW({
        const Primitive a = primitive_utils::convert(mx::Vector3{}, Type::Float);
    }, CompileError);

    ASSERT_THROW({
        const Primitive a = primitive_utils::convert(mx::Vector2{}, Type::Int);
    }, CompileError);

    ASSERT_THROW({
        const Primitive a = primitive_utils::convert(1, Type::String);
    }, CompileError);

    ASSERT_THROW({
        const Primitive a = primitive_utils::convert(mx::Vector3{}, Type::Mat3);
    }, CompileError);

    ASSERT_THROW({
        const Primitive a = primitive_utils::convert(mx::Matrix33{}, Type::Mat4);
    }, CompileError);
}

TEST(primitive_tests, test_extract)
{
    const float a = primitive_utils::extract(mx::Vector3{1, 2, 3}, 2).as<float>();
    ASSERT_FLOAT_EQ(a, 3);

    const float b = primitive_utils::extract(mx::Color4{0.75f, 0.5f, 1, 1}, 0).as<float>();
    ASSERT_FLOAT_EQ(b, 0.75f);

    const Primitive c = primitive_utils::extract(mx::Matrix33{}, 1);
    ASSERT_TRUE(c.is_a<mx::Vector3>());
}

TEST(primitive_tests, test_extract_fails)
{
    ASSERT_THROW({
        const Primitive a = primitive_utils::extract("hello", 2);
    }, CompileError);

    ASSERT_THROW({
        const Primitive a = primitive_utils::extract(true, 0);
    }, CompileError);

    ASSERT_THROW({
        const Primitive a = primitive_utils::extract(3.0f, 0);
    }, CompileError);

    ASSERT_THROW({
        const Primitive a = primitive_utils::extract(mx::Vector3{}, 0).as<int>();
    }, CompileError);

    ASSERT_THROW({
        const Primitive a = primitive_utils::extract(mx::Color3{}, 0).as<mx::Vector3>();
    }, CompileError);

    ASSERT_THROW({
        const Primitive a = primitive_utils::extract(mx::Matrix44{}, 0).as<mx::Vector3>();
    }, CompileError);

    ASSERT_THROW({
        const Primitive a = primitive_utils::extract(mx::Vector2{}, 2);
    }, CompileError);

    ASSERT_THROW({
        const Primitive a = primitive_utils::extract(mx::Matrix44{}, 10);
    }, CompileError);
}

TEST(primitive_tests, test_switch)
{
    const int a = primitive_utils::switch_({5, 3, 7}, 2).as<int>();
    ASSERT_EQ(a, 7);

    const float b = primitive_utils::switch_({0.75f, 0.5f, 1, 1}, 1).as<float>();
    ASSERT_FLOAT_EQ(b, 0.5f);

    const int c = primitive_utils::switch_({Primitive{}, 3, 7}, 0).as<int>();
    ASSERT_EQ(c, 0);

    const Primitive d = primitive_utils::switch_({Primitive{}}, 0, Type::Int);
    ASSERT_EQ(d, 0);
}

TEST(primitive_tests, test_switch_fails)
{
    ASSERT_THROW({
        const Primitive a = primitive_utils::switch_({5, 3, 7}, 5);
    }, CompileError);

    ASSERT_THROW({
        const Primitive a = primitive_utils::switch_({Primitive{}}, 0);
    }, CompileError);
}
