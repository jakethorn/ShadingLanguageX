//
// Created by jaket on 19/11/2026.
//

#include <algorithm>
#include <functional>

#include "gtest/gtest.h"

#include "common.h"
#include "CompileOptions.h"
#include "compile.h"
#include "reflection/reflect.h"
#include "runtime/Runtime.h"
#include "utils/container_utils.h"

using namespace mxslc;
using namespace mxslc::reflection;
using container_utils::contains;

namespace
{
    bool is_sorted_and_unique(const vector<string>& names)
    {
        return std::adjacent_find(names.begin(), names.end(), std::greater_equal<>{}) == names.end();
    }
}

TEST(reflection_tests, test_get_keywords)
{
    const vector<string> keywords = get_keywords();

    EXPECT_TRUE(is_sorted_and_unique(keywords));
    EXPECT_TRUE(contains(keywords, "if"));
    EXPECT_TRUE(contains(keywords, "node"));
    EXPECT_TRUE(contains(keywords, "xor"));
    // true and false are literals, not keywords
    EXPECT_FALSE(contains(keywords, "true"));
}

TEST(reflection_tests, test_reflect_type_names)
{
    const vector<string> type_names = reflect().type_names();

    EXPECT_TRUE(is_sorted_and_unique(type_names));
    // MaterialX type, stdlib alias and reserved type names
    EXPECT_TRUE(contains(type_names, "vector3"));
    EXPECT_TRUE(contains(type_names, "vec3"));
    EXPECT_TRUE(contains(type_names, "BSDF"));
    EXPECT_TRUE(contains(type_names, "void"));
    EXPECT_TRUE(contains(type_names, "auto"));
    EXPECT_TRUE(contains(type_names, "T"));
    EXPECT_FALSE(contains(type_names, "float3"));
}

TEST(reflection_tests, test_reflect_function_names)
{
    const vector<string> function_names = reflect().function_names();

    EXPECT_TRUE(is_sorted_and_unique(function_names));
    // MaterialX node and stdlib function
    EXPECT_TRUE(contains(function_names, "standard_surface"));
    EXPECT_TRUE(contains(function_names, "combine"));
    // keyword nodes and operator functions can't be called by name
    EXPECT_FALSE(contains(function_names, "and"));
    EXPECT_FALSE(contains(function_names, "out"));
    EXPECT_FALSE(contains(function_names, "__add__"));
    EXPECT_FALSE(contains(function_names, "sinh"));
}

TEST(reflection_tests, test_reflect_with_version)
{
    CompileOptions opts;
    opts.version = "1.38.10";

    const Reflection reflection = reflect(opts);

    // hextiledimage was added in 1.39
    EXPECT_FALSE(contains(reflection.function_names(), "hextiledimage"));
    EXPECT_TRUE(contains(reflect().function_names(), "hextiledimage"));
    EXPECT_TRUE(contains(reflection.type_names(), "vec3"));
}

TEST(reflection_tests, test_reflect_user_type_and_function_names)
{
    const string source = R"(
class Light
{
    vec3 pos;

    float brightness()
    {
        return 1.0;
    }
}

using Ray = {vec3 origin, vec3 dir};
using vector3 = float;

float add(float a, float b)
{
    return a - b;
}

float brighten(float x)
{
    return x * 2.0;
}
)";

    const Reflection reflection = reflect(source);

    EXPECT_TRUE(contains(reflection.type_names(), "Light"));
    EXPECT_TRUE(contains(reflection.type_names(), "Ray"));
    // the program's alias shadows the MaterialX type and isn't listed twice
    EXPECT_TRUE(is_sorted_and_unique(reflection.type_names()));
    EXPECT_TRUE(contains(reflection.function_names(), "brighten"));
    EXPECT_TRUE(contains(reflection.function_names(), "brightness"));
    // the program's overload of a MaterialX node isn't listed twice
    EXPECT_TRUE(is_sorted_and_unique(reflection.function_names()));
    EXPECT_FALSE(contains(reflect().function_names(), "brighten"));
}

TEST(reflection_tests, test_compile_does_not_reflect)
{
    const Reflection& reflection = compile(string{"float x = 1.0;"}).reflection();

    EXPECT_TRUE(reflection.type_names().empty());
    EXPECT_TRUE(reflection.function_names().empty());
}
