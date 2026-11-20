from pathlib import Path

import mxslc


def test_get_keywords():
    keywords = mxslc.get_keywords()

    assert keywords == sorted(keywords)
    assert {"if", "node", "and", "xor"} <= set(keywords)
    assert "true" not in keywords


def test_reflect_types():
    types = mxslc.reflect().types
    names = [t.name for t in types]

    assert isinstance(types[0], mxslc.TypeInfo)
    assert {"vector3", "vec3", "BSDF", "void", "auto", "T"} <= set(names)
    assert "float3" not in names


def test_reflect_functions():
    functions = mxslc.reflect().functions
    names = [f.name for f in functions]

    assert isinstance(functions[0], mxslc.FunctionInfo)
    assert {"add", "standard_surface", "combine"} <= set(names)
    assert not {"and", "out", "__add__", "sinh"} & set(names)


def test_reflect_with_version():
    opts = mxslc.CompileOptions(version = "1.38.10")

    assert "hextiledimage" not in [f.name for f in mxslc.reflect(opts).functions]
    assert "hextiledimage" in [f.name for f in mxslc.reflect().functions]


def test_reflect_user_types_and_functions():
    source = "class Light\n{\n    vec3 pos;\n}\n\nfloat brighten(float x)\n{\n    return x * 2.0;\n}\n"
    reflection = mxslc.reflect_from_string(source)

    assert "Light" in [t.name for t in reflection.types]
    assert "brighten" in [f.name for f in reflection.functions]
    assert "add" in [f.name for f in reflection.functions]


def test_reflect_aliases():
    source = "class Light\n{\n    vec3 pos;\n}\n\nusing Brightness = float;\n"
    types = {t.name: t for t in mxslc.reflect_from_string(source).types}

    assert types["vec3"].is_alias
    assert types["Brightness"].is_alias
    assert not types["vector3"].is_alias
    assert not types["Light"].is_alias
    assert not types["void"].is_alias


def test_reflect_builtin_types():
    source = "class Light\n{\n    vec3 pos;\n}\n\nusing Brightness = float;\n"
    types = {t.name: t for t in mxslc.reflect_from_string(source).types}

    assert types["float"].source == mxslc.Source.MaterialX
    assert types["vec3"].source == mxslc.Source.SlxStandardLibrary
    assert types["void"].source == mxslc.Source.SlxStandardLibrary
    assert types["Light"].source == mxslc.Source.UserDefined
    assert types["Brightness"].source == mxslc.Source.UserDefined


def test_reflect_type_locations():
    opts = mxslc.CompileOptions()
    opts.add_source("types.mxsl", "class Light\n{\n    vec3 pos;\n}\n")
    source = '#include "types.mxsl"\nusing Brightness = float;\n'
    types = {t.name: t for t in mxslc.reflect_from_string(source, opts).types}

    assert (types["Light"].path, types["Light"].line) == (Path("types.mxsl"), 1)
    assert (types["Brightness"].path, types["Brightness"].line) == (None, 2)
    assert types["vec3"].path.is_absolute() and types["vec3"].path.name == "stdlib.mxsl"
    assert types["float"].path.name == "stdlib_defs.mtlx" and types["float"].line is None
    assert (types["void"].path, types["void"].line) == (None, None)


def test_reflect_type_paths_from_file(tmp_path):
    (tmp_path / "light.mxsl").write_text("class Light\n{\n    vec3 pos;\n}\n")
    (tmp_path / "types.mxsl").write_text('#include "light.mxsl"\n\nusing Brightness = float;\n')
    types = {t.name: t for t in mxslc.reflect_from_file(tmp_path / "types.mxsl").types}

    assert (types["Brightness"].path, types["Brightness"].line) == (tmp_path / "types.mxsl", 3)
    assert (types["Light"].path, types["Light"].line) == (tmp_path / "light.mxsl", 1)


def test_reflect_aliased_types_fields_and_methods():
    source = """
class Light
{
    vec3 pos;
    float intensity;

    float brightness()
    {
        return intensity * 2.0;
    }
}

using Lamp = Light;
using Ray = {vec3 origin, vec3 dir};
"""
    types = {t.name: t for t in mxslc.reflect_from_string(source).types}

    assert types["vec3"].aliased_type == "vector3"
    assert types["Ray"].aliased_type == "{vector3 origin, vector3 dir}"
    assert types["Light"].aliased_type is None
    assert [(f.name, f.type) for f in types["Light"].fields] == [("pos", "vector3"), ("intensity", "float")]
    assert isinstance(types["Light"].fields[0], mxslc.FieldInfo)
    assert [m.name for m in types["Light"].methods] == ["brightness"]
    assert [m.name for m in types["Lamp"].methods] == ["brightness"]
    assert [(f.name, f.type) for f in types["Ray"].fields] == [("origin", "vector3"), ("dir", "vector3")]


def test_reflect_materialx_node_overloads():
    functions = {f.name: f for f in mxslc.reflect().functions}
    add = next(o for o in functions["add"].overloads if o.source == mxslc.Source.MaterialX and o.template_type == "float")
    image = next(o for o in functions["image"].overloads if o.template_type == "float")
    parameters = {p.name: p for p in image.parameters}

    assert isinstance(add, mxslc.OverloadInfo) and isinstance(add.parameters[0], mxslc.ParameterInfo)
    assert [(p.name, p.type, p.default_expression) for p in add.parameters] == [("in1", "float", "0.0"), ("in2", "float", "0.0")]
    assert (add.return_type, add.template_type, add.modifiers) == ("float", "float", [])
    assert add.path.name == "stdlib_defs.mtlx" and add.line is None
    assert parameters["file"].modifiers == ["uniform"]
    assert parameters["texcoord"].default_expression == "geomprop(UV0)"


def test_reflect_function_overloads():
    source = """
float add(float a, float b)
{
    return a - b;
}

inline float twice(uniform float x = 1.0)
{
    return x * 2.0;
}

class Light
{
    float intensity;

    float brightness()
    {
        return intensity;
    }
}
"""
    functions = {f.name: f for f in mxslc.reflect_from_string(source).functions}
    twice = functions["twice"].overloads[0]

    assert twice.modifiers == ["inline"]
    assert [(p.name, p.default_expression, p.modifiers) for p in twice.parameters] == [("x", "1.0", ["uniform"])]
    assert twice.source == mxslc.Source.UserDefined
    assert (twice.template_type, twice.class_type, twice.path, twice.line) == (None, None, None, 7)
    assert functions["brightness"].overloads[0].class_type == "Light"
    float_adds = [o for o in functions["add"].overloads if [p.type for p in o.parameters] == ["float", "float"]]
    assert [o.source for o in float_adds] == [mxslc.Source.UserDefined]
    assert any(o.template_type == "vector3" for o in functions["add"].overloads)
