//
// Created by jaket on 12/07/2026.
//

#include "gtest/gtest.h"
#include "Primitive.h"
#include "runtime/Type.h"

using namespace mxslc;

TEST(primitive_tests, test_is_a)
{
    ASSERT_TRUE(Primitive{true}.is_a<bool>());
    ASSERT_TRUE(Primitive{false}.is_a<bool>());
    ASSERT_FALSE(Primitive{true}.is_a<int>());
    ASSERT_FALSE(Primitive{false}.is_a<float>());

    ASSERT_TRUE(Primitive{0}.is_a<int>());
    ASSERT_TRUE(Primitive{1}.is_a<int>());
    ASSERT_TRUE(Primitive{2}.is_a<int>());
    ASSERT_TRUE(Primitive{700}.is_a<int>());
    ASSERT_TRUE(Primitive{14000}.is_a<int>());
    ASSERT_FALSE(Primitive{0}.is_a<bool>());
    ASSERT_FALSE(Primitive{1}.is_a<float>());

    ASSERT_TRUE(Primitive{1.4f}.is_a<float>());
    ASSERT_TRUE(Primitive{.1f}.is_a<float>());
    ASSERT_TRUE(Primitive{.01f}.is_a<float>());
    ASSERT_TRUE(Primitive{7000.01f}.is_a<float>());
    ASSERT_FALSE(Primitive{0.0f}.is_a<bool>());
    ASSERT_FALSE(Primitive{1.0f}.is_a<int>());

    ASSERT_TRUE(Primitive{"hello"}.is_a<string>());
    ASSERT_TRUE(Primitive{"world"s}.is_a<string>());
    ASSERT_TRUE(Primitive{""s}.is_a<string>());
    ASSERT_TRUE(Primitive{"this is a long string with spaces in it to test the primitive class"s}.is_a<string>());
    ASSERT_FALSE(Primitive{"image.png"}.is_a<fs::path>());
    ASSERT_FALSE(Primitive{"image.png"s}.is_a<fs::path>());

    ASSERT_TRUE(Primitive{fs::path{"hello"}}.is_a<fs::path>());
    ASSERT_TRUE(Primitive{fs::path{"image.png"}}.is_a<fs::path>());
    ASSERT_FALSE(Primitive{fs::path{"world"}}.is_a<string>());

    ASSERT_TRUE(Primitive{mx::Vector2()}.is_a<mx::Vector2>());
    std::array<float, 2> a2 = {0, 0};
    ASSERT_TRUE(Primitive{a2}.is_a<mx::Vector2>());

    ASSERT_TRUE(Primitive{mx::Vector3()}.is_a<mx::Vector3>());
    std::array<float, 3> a3 = {0, 0, 0};
    ASSERT_TRUE(Primitive{a3}.is_a<mx::Vector3>());

    ASSERT_TRUE(Primitive{mx::Vector4()}.is_a<mx::Vector4>());
    std::array<float, 4> a4 = {0, 0, 0, 0};
    ASSERT_TRUE(Primitive{a4}.is_a<mx::Vector4>());

    ASSERT_TRUE(Primitive{mx::Color3()}.is_a<mx::Color3>());
    ASSERT_TRUE(Primitive{mx::Color4()}.is_a<mx::Color4>());
    ASSERT_TRUE(Primitive{mx::Matrix33()}.is_a<mx::Matrix33>());
    ASSERT_TRUE(Primitive{mx::Matrix44()}.is_a<mx::Matrix44>());
}

TEST(primitive_tests, test_type)
{
    ASSERT_TRUE(Primitive{true}.type()->is<bool>());
    ASSERT_TRUE(Primitive{false}.type()->is<bool>());
    ASSERT_FALSE(Primitive{true}.type()->is<int>());
    ASSERT_FALSE(Primitive{false}.type()->is<float>());

    ASSERT_TRUE(Primitive{0}.type()->is<int>());
    ASSERT_TRUE(Primitive{1}.type()->is<int>());
    ASSERT_TRUE(Primitive{2}.type()->is<int>());
    ASSERT_TRUE(Primitive{700}.type()->is<int>());
    ASSERT_TRUE(Primitive{14000}.type()->is<int>());
    ASSERT_FALSE(Primitive{0}.type()->is<bool>());
    ASSERT_FALSE(Primitive{1}.type()->is<float>());

    ASSERT_TRUE(Primitive{1.4f}.type()->is<float>());
    ASSERT_TRUE(Primitive{.1f}.type()->is<float>());
    ASSERT_TRUE(Primitive{.01f}.type()->is<float>());
    ASSERT_TRUE(Primitive{7000.01f}.type()->is<float>());
    ASSERT_FALSE(Primitive{0.0f}.type()->is<bool>());
    ASSERT_FALSE(Primitive{1.0f}.type()->is<int>());

    ASSERT_TRUE(Primitive{"hello"}.type()->is<string>());
    ASSERT_TRUE(Primitive{"world"s}.type()->is<string>());
    ASSERT_TRUE(Primitive{""s}.type()->is<string>());
    ASSERT_TRUE(Primitive{"this is a long string with spaces in it to test the primitive class"s}.type()->is<string>());
    ASSERT_FALSE(Primitive{"image.png"}.type()->is<fs::path>());
    ASSERT_FALSE(Primitive{"image.png"s}.type()->is<fs::path>());

    ASSERT_TRUE(Primitive{fs::path{"hello"}}.type()->is<fs::path>());
    ASSERT_TRUE(Primitive{fs::path{"image.png"}}.type()->is<fs::path>());
    ASSERT_FALSE(Primitive{fs::path{"world"}}.type()->is<string>());

    ASSERT_TRUE(Primitive{mx::Vector2()}.type()->is<mx::Vector2>());
    std::array<float, 2> a2 = {0, 0};
    ASSERT_TRUE(Primitive{a2}.type()->is<mx::Vector2>());

    ASSERT_TRUE(Primitive{mx::Vector3()}.type()->is<mx::Vector3>());
    std::array<float, 3> a3 = {0, 0, 0};
    ASSERT_TRUE(Primitive{a3}.type()->is<mx::Vector3>());

    ASSERT_TRUE(Primitive{mx::Vector4()}.type()->is<mx::Vector4>());
    std::array<float, 4> a4 = {0, 0, 0, 0};
    ASSERT_TRUE(Primitive{a4}.type()->is<mx::Vector4>());

    ASSERT_TRUE(Primitive{mx::Color3()}.type()->is<mx::Color3>());
    ASSERT_TRUE(Primitive{mx::Color4()}.type()->is<mx::Color4>());
    ASSERT_TRUE(Primitive{mx::Matrix33()}.type()->is<mx::Matrix33>());
    ASSERT_TRUE(Primitive{mx::Matrix44()}.type()->is<mx::Matrix44>());
}

TEST(primitive_tests, test_type_name)
{
    ASSERT_TRUE(Primitive{true}.type_name() == "boolean");
    ASSERT_TRUE(Primitive{false}.type_name() == "boolean");
    ASSERT_FALSE(Primitive{true}.type_name() == "integer");
    ASSERT_FALSE(Primitive{false}.type_name() == "float");

    ASSERT_TRUE(Primitive{0}.type_name() == "integer");
    ASSERT_TRUE(Primitive{1}.type_name() == "integer");
    ASSERT_TRUE(Primitive{2}.type_name() == "integer");
    ASSERT_TRUE(Primitive{700}.type_name() == "integer");
    ASSERT_TRUE(Primitive{14000}.type_name() == "integer");
    ASSERT_FALSE(Primitive{0}.type_name() == "boolean");
    ASSERT_FALSE(Primitive{1}.type_name() == "float");

    ASSERT_TRUE(Primitive{1.4f}.type_name() == "float");
    ASSERT_TRUE(Primitive{.1f}.type_name() == "float");
    ASSERT_TRUE(Primitive{.01f}.type_name() == "float");
    ASSERT_TRUE(Primitive{7000.01f}.type_name() == "float");
    ASSERT_FALSE(Primitive{0.0f}.type_name() == "boolean");
    ASSERT_FALSE(Primitive{1.0f}.type_name() == "integer");

    ASSERT_TRUE(Primitive{"hello"}.type_name() == "string");
    ASSERT_TRUE(Primitive{"world"s}.type_name() == "string");
    ASSERT_TRUE(Primitive{""s}.type_name() == "string");
    ASSERT_TRUE(Primitive{"this is a long string with spaces in it to test the primitive class"s}.type_name() == "string");
    ASSERT_FALSE(Primitive{"image.png"}.type_name() == "filename");
    ASSERT_FALSE(Primitive{"image.png"s}.type_name() == "filename");

    ASSERT_TRUE(Primitive{fs::path{"hello"}}.type_name() == "filename");
    ASSERT_TRUE(Primitive{fs::path{"image.png"}}.type_name() == "filename");
    ASSERT_FALSE(Primitive{fs::path{"world"}}.type_name() == "string");

    ASSERT_TRUE(Primitive{mx::Vector2()}.type_name() == "vector2");
    std::array<float, 2> a2 = {0, 0};
    ASSERT_TRUE(Primitive{a2}.type_name() == "vector2");

    ASSERT_TRUE(Primitive{mx::Vector3()}.type_name() == "vector3");
    std::array<float, 3> a3 = {0, 0, 0};
    ASSERT_TRUE(Primitive{a3}.type_name() == "vector3");

    ASSERT_TRUE(Primitive{mx::Vector4()}.type_name() == "vector4");
    std::array<float, 4> a4 = {0, 0, 0, 0};
    ASSERT_TRUE(Primitive{a4}.type_name() == "vector4");

    ASSERT_TRUE(Primitive{mx::Color3()}.type_name() == "color3");
    ASSERT_TRUE(Primitive{mx::Color4()}.type_name() == "color4");
    ASSERT_TRUE(Primitive{mx::Matrix33()}.type_name() == "matrix33");
    ASSERT_TRUE(Primitive{mx::Matrix44()}.type_name() == "matrix44");
}

namespace
{
    template<typename T>
    Primitive prim()
    {
        return Primitive{T{}};
    }

    template<typename T, typename... Ts>
    bool is_castable_to()
    {
        return (prim<T>().template is_castable_to<Ts>() and ...);
    }

    template<typename T, typename... Ts>
    bool is_not_castable_to()
    {
        return not (prim<T>().template is_castable_to<Ts>() or ...);
    }

    template<typename T>
    bool is_castable_to_self()
    {
        return prim<T>().template is_castable_to<T>();
    }
}

TEST(primitive_tests, test_is_castable_to)
{
#define ASSERT_IS_CASTABLE_TO_SELF(type) ASSERT_TRUE(is_castable_to_self<type>());
    FOR_EACH_PRIMITIVE_TYPE(ASSERT_IS_CASTABLE_TO_SELF, )
#undef ASSERT_IS_CASTABLE_TO_SELF

    ASSERT_TRUE((is_castable_to<bool, int, float, mx::Vector2, mx::Vector3, mx::Vector4, mx::Color3, mx::Color4, mx::Matrix33, mx::Matrix44>()));
    ASSERT_TRUE((is_castable_to<int, bool, float, mx::Vector2, mx::Vector3, mx::Vector4, mx::Color3, mx::Color4, mx::Matrix33, mx::Matrix44>()));
    ASSERT_TRUE((is_castable_to<float, bool, int, mx::Vector2, mx::Vector3, mx::Vector4, mx::Color3, mx::Color4, mx::Matrix33, mx::Matrix44>()));

    ASSERT_TRUE((is_castable_to<string, fs::path>()));
    ASSERT_TRUE((is_castable_to<fs::path, string>()));

    ASSERT_TRUE((is_not_castable_to<bool, string, fs::path>()));
    ASSERT_TRUE((is_not_castable_to<int, string, fs::path>()));
    ASSERT_TRUE((is_not_castable_to<float, string, fs::path>()));
    ASSERT_TRUE((is_not_castable_to<mx::Vector2, mx::Vector3, mx::Vector4, mx::Color3, mx::Color4, mx::Matrix33, mx::Matrix44, string, fs::path>()));
    ASSERT_TRUE((is_not_castable_to<mx::Vector3, mx::Vector2, mx::Vector4, mx::Color3, mx::Color4, mx::Matrix33, mx::Matrix44, string, fs::path>()));
    ASSERT_TRUE((is_not_castable_to<mx::Vector4, mx::Vector2, mx::Vector3, mx::Color3, mx::Color4, mx::Matrix33, mx::Matrix44, string, fs::path>()));
    ASSERT_TRUE((is_not_castable_to<mx::Color3, mx::Vector2, mx::Vector3, mx::Vector4, mx::Color4, mx::Matrix33, mx::Matrix44, string, fs::path>()));
    ASSERT_TRUE((is_not_castable_to<mx::Color4, mx::Vector2, mx::Vector3, mx::Vector4, mx::Color3, mx::Matrix33, mx::Matrix44, string, fs::path>()));
    ASSERT_TRUE((is_not_castable_to<mx::Matrix33, mx::Vector2, mx::Vector3, mx::Vector4, mx::Color3, mx::Color4, mx::Matrix44, string, fs::path>()));
    ASSERT_TRUE((is_not_castable_to<mx::Matrix44, mx::Vector2, mx::Vector3, mx::Vector4, mx::Color3, mx::Color4, mx::Matrix33, string, fs::path>()));

    ASSERT_TRUE((is_not_castable_to<string, bool, int, float, mx::Vector2, mx::Vector3, mx::Vector4, mx::Color3, mx::Color4, mx::Matrix33, mx::Matrix44>()));
    ASSERT_TRUE((is_not_castable_to<fs::path, bool, int, float, mx::Vector2, mx::Vector3, mx::Vector4, mx::Color3, mx::Color4, mx::Matrix33, mx::Matrix44>()));
}

TEST(primitive_tests, test_as)
{
#define PRIM_AS_SELF(type) prim<type>().as<type>();
    ASSERT_NO_THROW({
        FOR_EACH_PRIMITIVE_TYPE(PRIM_AS_SELF, )
    });
#undef PRIM_AS_SELF
}

TEST(primitive_tests, test_as_fails)
{
    ASSERT_THROW({
        prim<bool>().as<int>();
    }, mxslc::CompileError);

    ASSERT_THROW({
        prim<float>().as<int>();
    }, mxslc::CompileError);

    ASSERT_THROW({
        prim<int>().as<float>();
    }, mxslc::CompileError);

    ASSERT_THROW({
        prim<float>().as<mx::Vector3>();
    }, mxslc::CompileError);

    ASSERT_THROW({
        prim<string>().as<fs::path>();
    }, mxslc::CompileError);

    ASSERT_THROW({
        prim<mx::Vector2>().as<mx::Vector3>();
    }, mxslc::CompileError);

    ASSERT_THROW({
        prim<mx::Vector2>().as<float>();
    }, mxslc::CompileError);

    ASSERT_THROW({
        prim<mx::Color3>().as<string>();
    }, mxslc::CompileError);
}

TEST(primitive_tests, test_cast)
{
#define PRIM_CAST_SELF(type) prim<type>().cast<type>();
    ASSERT_NO_THROW({
        FOR_EACH_PRIMITIVE_TYPE(PRIM_CAST_SELF, )
    });
#undef PRIM_CAST_SELF

    ASSERT_NO_THROW({
        prim<bool>().cast<int>();
        prim<bool>().cast<float>();
        prim<int>().cast<bool>();
        prim<int>().cast<float>();
        prim<float>().cast<bool>();
        prim<float>().cast<int>();
        prim<string>().cast<fs::path>();
        prim<fs::path>().cast<string>();
        prim<bool>().cast<mx::Vector2>();
        prim<int>().cast<mx::Vector4>();
        prim<float>().cast<mx::Color3>();
    });
}

TEST(primitive_tests, test_cast_fails)
{
    ASSERT_THROW({
        prim<mx::Vector2>().as<mx::Vector3>();
    }, mxslc::CompileError);

    ASSERT_THROW({
        prim<mx::Vector2>().as<float>();
    }, mxslc::CompileError);

    ASSERT_THROW({
        prim<mx::Color3>().as<string>();
    }, mxslc::CompileError);

    ASSERT_THROW({
        prim<mx::Vector4>().as<mx::Color4>();
    }, mxslc::CompileError);

}
