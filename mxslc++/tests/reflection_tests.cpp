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
#include "utils/data_utils.h"

using namespace mxslc;
using namespace mxslc::reflection;
using container_utils::contains;

namespace
{
    vector<string> get_names(const vector<TypeInfo>& types)
    {
        vector<string> names;
        for (const TypeInfo& type : types)
            names.push_back(type.name);
        return names;
    }

    vector<string> get_names(const vector<FunctionInfo>& functions)
    {
        vector<string> names;
        for (const FunctionInfo& function : functions)
            names.push_back(function.name);
        return names;
    }

    vector<string> get_alias_names(const vector<TypeInfo>& types)
    {
        vector<string> names;
        for (const TypeInfo& type : types)
            if (type.is_alias)
                names.push_back(type.name);
        return names;
    }

    TypeInfo get_type_info(const vector<TypeInfo>& types, const string& name)
    {
        const auto it = std::find_if(types.begin(), types.end(), [&](const TypeInfo& type) { return type.name == name; });
        return it != types.end() ? *it : TypeInfo{};
    }

    vector<string> get_field_declarations(const vector<FieldInfo>& fields)
    {
        vector<string> declarations;
        for (const FieldInfo& field : fields)
            declarations.push_back(field.type + " " + field.name);
        return declarations;
    }

    bool is_absolute_path_to(const fs::path& path, const string& filename)
    {
        return path.is_absolute() and path.filename() == filename;
    }

    string normalise_path(const fs::path& path)
    {
        return path.lexically_normal().string();
    }

    FunctionInfo get_function_info(const vector<FunctionInfo>& functions, const string& name)
    {
        const auto it = std::find_if(functions.begin(), functions.end(), [&](const FunctionInfo& function) { return function.name == name; });
        return it != functions.end() ? *it : FunctionInfo{};
    }

    OverloadInfo get_materialx_overload(const vector<FunctionInfo>& functions, const string& name, const string& template_type)
    {
        for (const OverloadInfo& overload : get_function_info(functions, name).overloads)
            if (overload.source == Source::MaterialX and overload.template_type == template_type)
                return overload;
        return OverloadInfo{};
    }

    vector<string> get_parameter_declarations(const OverloadInfo& overload)
    {
        vector<string> declarations;
        for (const ParameterInfo& parameter : overload.parameters)
        {
            string declaration;
            for (const string& modifier : parameter.modifiers)
                declaration += modifier + " ";
            declaration += parameter.type + " " + parameter.name;
            if (not parameter.default_expression.empty())
                declaration += " = " + parameter.default_expression;
            declarations.push_back(declaration);
        }
        return declarations;
    }

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

TEST(reflection_tests, test_reflect_types)
{
    const vector<string> type_names = get_names(reflect().types());

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

TEST(reflection_tests, test_reflect_functions)
{
    const vector<string> function_names = get_names(reflect().functions());

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
    EXPECT_FALSE(contains(get_names(reflection.functions()), "hextiledimage"));
    EXPECT_TRUE(contains(get_names(reflect().functions()), "hextiledimage"));
    EXPECT_TRUE(contains(get_names(reflection.types()), "vec3"));
}

TEST(reflection_tests, test_reflect_user_types_and_functions)
{
    const string source = R"(
class Light
{
    vec3 pos;
}

using Ray = {vec3 origin, vec3 dir};

float brighten(float x)
{
    return x * 2.0;
}
)";

    const Reflection reflection = reflect(source);
    const vector<string> type_names = get_names(reflection.types());
    const vector<string> function_names = get_names(reflection.functions());

    EXPECT_TRUE(contains(type_names, "Light"));
    EXPECT_TRUE(contains(type_names, "Ray"));
    EXPECT_TRUE(contains(type_names, "vec3"));
    EXPECT_TRUE(contains(function_names, "brighten"));
    EXPECT_TRUE(contains(function_names, "add"));
    EXPECT_FALSE(contains(get_names(reflect().functions()), "brighten"));
}

TEST(reflection_tests, test_reflect_aliases)
{
    const vector<string> stdlib_alias_names = get_alias_names(reflect().types());

    EXPECT_TRUE(contains(stdlib_alias_names, "vec3"));
    EXPECT_FALSE(contains(stdlib_alias_names, "vector3"));
    EXPECT_FALSE(contains(stdlib_alias_names, "void"));
    EXPECT_FALSE(contains(stdlib_alias_names, "T"));

    const string source = R"(
class Light
{
    vec3 pos;
}

using Ray = {vec3 origin, vec3 dir};
using Brightness = float;
using vector3 = float;
)";

    const Reflection reflection = reflect(source);
    const vector<string> type_names = get_names(reflection.types());
    const vector<string> alias_names = get_alias_names(reflection.types());

    EXPECT_TRUE(is_sorted_and_unique(type_names));
    EXPECT_FALSE(contains(alias_names, "Light"));
    // an alias of an unnamed type
    EXPECT_TRUE(contains(alias_names, "Ray"));
    EXPECT_TRUE(contains(alias_names, "Brightness"));
    // the program's alias shadows the MaterialX type
    EXPECT_TRUE(contains(alias_names, "vector3"));
}

TEST(reflection_tests, test_reflect_type_sources)
{
    const string source = R"(
class Light
{
    vec3 pos;
}

using Brightness = float;
using vector3 = float;
)";

    const vector<TypeInfo> types = reflect(source).types();

    EXPECT_EQ(get_type_info(types, "float").source, Source::MaterialX);
    EXPECT_EQ(get_type_info(types, "vec3").source, Source::SlxStandardLibrary);
    // reserved types are part of SLX
    EXPECT_EQ(get_type_info(types, "void").source, Source::SlxStandardLibrary);
    EXPECT_EQ(get_type_info(types, "Light").source, Source::UserDefined);
    // the program's alias of a MaterialX type
    EXPECT_EQ(get_type_info(types, "Brightness").source, Source::UserDefined);
    // the program's alias shadows the MaterialX type
    EXPECT_EQ(get_type_info(types, "vector3").source, Source::UserDefined);
}

TEST(reflection_tests, test_reflect_type_locations)
{
    const string source = R"(#include "types.mxsl"
using Brightness = float;
using vector3 = float;
)";

    CompileOptions opts;
    opts.add_source("types.mxsl", "class Light\n{\n    vec3 pos;\n}\n");

    const vector<TypeInfo> types = reflect(source, opts).types();

    EXPECT_EQ(get_type_info(types, "Light").path.string(), "types.mxsl");
    EXPECT_EQ(get_type_info(types, "Light").line, 1);
    // a source compiled from a string has no path
    EXPECT_TRUE(get_type_info(types, "Brightness").path.empty());
    EXPECT_EQ(get_type_info(types, "Brightness").line, 2);
    // the program's alias shadows the MaterialX type
    EXPECT_EQ(get_type_info(types, "vector3").line, 3);
    EXPECT_TRUE(is_absolute_path_to(get_type_info(types, "vec3").path, "stdlib.mxsl"));
    EXPECT_GT(get_type_info(types, "vec3").line, 0);
    // MaterialX doesn't record line numbers
    EXPECT_TRUE(is_absolute_path_to(get_type_info(types, "float").path, "stdlib_defs.mtlx"));
    EXPECT_EQ(get_type_info(types, "float").line, 0);
    EXPECT_TRUE(get_type_info(types, "void").path.empty());
    EXPECT_EQ(get_type_info(types, "void").line, 0);
}

TEST(reflection_tests, test_reflect_type_paths_from_file)
{
    const fs::path types_path = get_test_data("reflection/types.mxsl");
    const fs::path light_path = get_test_data("reflection/light.mxsl");

    const vector<TypeInfo> types = reflect(types_path).types();

    EXPECT_EQ(normalise_path(get_type_info(types, "Brightness").path), normalise_path(types_path));
    EXPECT_EQ(get_type_info(types, "Brightness").line, 3);
    // an included file
    EXPECT_EQ(normalise_path(get_type_info(types, "Light").path), normalise_path(light_path));
    EXPECT_EQ(get_type_info(types, "Light").line, 1);
}

TEST(reflection_tests, test_reflect_aliased_types)
{
    const string source = R"(
using Brightness = float;
using Ray = {vec3 origin, vec3 dir};
using v3 = vec3;
)";

    const vector<TypeInfo> types = reflect(source).types();

    EXPECT_EQ(get_type_info(types, "vec3").aliased_type, "vector3");
    EXPECT_EQ(get_type_info(types, "Brightness").aliased_type, "float");
    EXPECT_EQ(get_type_info(types, "Ray").aliased_type, "{vector3 origin, vector3 dir}");
    // an alias of an alias is an alias of its type
    EXPECT_EQ(get_type_info(types, "v3").aliased_type, "vector3");
    EXPECT_EQ(get_type_info(types, "vector3").aliased_type, "");
}

TEST(reflection_tests, test_reflect_fields_and_methods)
{
    const string source = R"(
class Light
{
    vec3 pos;
    float intensity;

    Light(vec3 pos)
    {
        this.pos = pos;
        this.intensity = 1.0;
    }

    float brightness()
    {
        return intensity * 2.0;
    }

    float brightness(float scale)
    {
        return intensity * scale;
    }
}

using Lamp = Light;
using Ray = {vec3 origin, vec3 dir};
)";

    const vector<TypeInfo> types = reflect(source).types();
    const vector<string> light_fields{"vector3 pos", "float intensity"};

    EXPECT_EQ(get_field_declarations(get_type_info(types, "Light").fields), light_fields);
    // one entry for the overloads, and no constructor
    EXPECT_EQ(get_names(get_type_info(types, "Light").methods), vector<string>{"brightness"});
    EXPECT_EQ(get_field_declarations(get_type_info(types, "Lamp").fields), light_fields);
    EXPECT_EQ(get_names(get_type_info(types, "Lamp").methods), vector<string>{"brightness"});
    EXPECT_EQ(get_field_declarations(get_type_info(types, "Ray").fields), (vector<string>{"vector3 origin", "vector3 dir"}));
    EXPECT_TRUE(get_type_info(types, "Ray").methods.empty());
    EXPECT_TRUE(get_type_info(types, "vector3").fields.empty());
}

TEST(reflection_tests, test_reflect_materialx_node_overloads)
{
    const vector<FunctionInfo> functions = reflect().functions();

    const OverloadInfo add = get_materialx_overload(functions, "add", "float");
    EXPECT_EQ(get_parameter_declarations(add), (vector<string>{"float in1 = 0.0", "float in2 = 0.0"}));
    EXPECT_EQ(add.return_type, "float");
    EXPECT_TRUE(add.modifiers.empty());
    EXPECT_TRUE(add.class_type.empty());
    EXPECT_TRUE(is_absolute_path_to(add.path, "stdlib_defs.mtlx"));
    // MaterialX doesn't record line numbers
    EXPECT_EQ(add.line, 0);

    const vector<string> image_parameters = get_parameter_declarations(get_materialx_overload(functions, "image", "float"));
    EXPECT_TRUE(contains(image_parameters, "uniform filename file = \"\""));
    EXPECT_TRUE(contains(image_parameters, "vector2 texcoord = geomprop(UV0)"));

    EXPECT_EQ(get_materialx_overload(functions, "separate3", "color3").return_type, "{float outr, float outg, float outb}");
    // the default NodeDef of a node
    EXPECT_EQ(get_materialx_overload(functions, "add", "vector3").modifiers, vector<string>{"default"});
}

TEST(reflection_tests, test_reflect_function_overloads)
{
    const string source = R"(
inline float a()
{
    return 1.0;
}

nodegraph float b(float x = 1.0)
{
    return x;
}

comptime float c(float x)
{
    return x;
}

float d(uniform float x, float y = 2.0)
{
    return x * y;
}

void e(float x, out float y)
{
    y = x;
}

inline T twice<float, vec3>(T x)
{
    return x * 2.0;
}
)";

    const vector<FunctionInfo> functions = reflect(source).functions();

    EXPECT_EQ(get_function_info(functions, "a").overloads[0].modifiers, vector<string>{"inline"});
    EXPECT_EQ(get_function_info(functions, "b").overloads[0].modifiers, vector<string>{"nodegraph"});
    EXPECT_EQ(get_function_info(functions, "c").overloads[0].modifiers, vector<string>{"comptime"});

    const OverloadInfo d = get_function_info(functions, "d").overloads[0];
    EXPECT_TRUE(d.modifiers.empty());
    EXPECT_EQ(get_parameter_declarations(d), (vector<string>{"uniform float x", "float y = 2.0"}));
    EXPECT_EQ(d.source, Source::UserDefined);
    EXPECT_EQ(d.line, 17);

    const OverloadInfo e = get_function_info(functions, "e").overloads[0];
    EXPECT_EQ(e.return_type, "void");
    // out parameters are also mutable
    EXPECT_EQ(get_parameter_declarations(e), (vector<string>{"float x", "mutable out float y"}));

    const vector<OverloadInfo> twice = get_function_info(functions, "twice").overloads;
    ASSERT_EQ(twice.size(), 2);
    EXPECT_EQ(twice[0].template_type, "float");
    EXPECT_EQ(twice[1].template_type, "vector3");
    EXPECT_EQ(twice[1].return_type, "vector3");
}

TEST(reflection_tests, test_reflect_stdlib_function_and_method_overloads)
{
    const string source = R"(
class Light
{
    float intensity;

    float brightness(float scale)
    {
        return intensity * scale;
    }
}
)";

    const Reflection reflection = reflect(source);

    const OverloadInfo combine = get_function_info(reflection.functions(), "combine").overloads[0];
    EXPECT_EQ(combine.source, Source::SlxStandardLibrary);
    EXPECT_TRUE(is_absolute_path_to(combine.path, "stdlib.mxsl"));
    EXPECT_GT(combine.line, 0);

    const OverloadInfo method = get_type_info(reflection.types(), "Light").methods[0].overloads[0];
    EXPECT_EQ(method.class_type, "Light");
    EXPECT_EQ(method.line, 6);
    EXPECT_EQ(get_function_info(reflection.functions(), "brightness").overloads[0].class_type, "Light");
}

TEST(reflection_tests, test_reflect_shadowed_overloads)
{
    const string source = R"(
float add(float a, float b)
{
    return a - b;
}
)";

    const vector<FunctionInfo> functions = reflect(source).functions();
    const OverloadInfo program_add = get_function_info(functions, "add").overloads[0];

    EXPECT_EQ(program_add.source, Source::UserDefined);
    EXPECT_EQ(get_parameter_declarations(program_add), (vector<string>{"float a", "float b"}));
    // the program's overload shadows the MaterialX overload with the same signature, but not the others
    EXPECT_TRUE(get_materialx_overload(functions, "add", "float").return_type.empty());
    EXPECT_EQ(get_materialx_overload(functions, "add", "vector3").return_type, "vector3");
}

TEST(reflection_tests, test_compile_does_not_reflect)
{
    const Reflection& reflection = compile(string{"float x = 1.0;"}).reflection();

    EXPECT_TRUE(reflection.types().empty());
    EXPECT_TRUE(reflection.functions().empty());
}

TEST(reflection_tests, test_reflect_default_function_behaviour_modifier)
{
    CompileOptions opts;
    opts.default_function_behaviour = FunctionBehaviour::Inline;

    const vector<FunctionInfo> functions = reflect(string{"float f(float x)\n{\n    return x;\n}\n"}, opts).functions();

    // a function without a behaviour modifier gets the default behaviour's
    EXPECT_EQ(get_function_info(functions, "f").overloads[0].modifiers, vector<string>{"inline"});
}
