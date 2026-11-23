import mxslc


def test_get_keywords():
    keywords = mxslc.get_keywords()

    assert keywords == sorted(keywords)
    assert {"if", "node", "and", "xor"} <= set(keywords)
    assert "true" not in keywords


def test_reflect_type_names():
    type_names = mxslc.reflect().type_names

    assert type_names == sorted(set(type_names))
    assert {"vector3", "vec3", "BSDF", "void", "auto", "T"} <= set(type_names)
    assert "float3" not in type_names


def test_reflect_function_names():
    function_names = mxslc.reflect().function_names

    assert function_names == sorted(set(function_names))
    assert {"add", "standard_surface", "combine"} <= set(function_names)
    assert not {"and", "out", "__add__", "sinh"} & set(function_names)


def test_reflect_with_version():
    opts = mxslc.CompileOptions(version = "1.38.10")

    assert "hextiledimage" not in mxslc.reflect(opts).function_names
    assert "hextiledimage" in mxslc.reflect().function_names


def test_reflect_user_type_and_function_names():
    source = "class Light\n{\n    vec3 pos;\n}\n\nusing Ray = {vec3 origin, vec3 dir};\n\nfloat brighten(float x)\n{\n    return x * 2.0;\n}\n"
    reflection = mxslc.reflect_from_string(source)

    assert {"Light", "Ray"} <= set(reflection.type_names)
    assert {"brighten", "add"} <= set(reflection.function_names)


def test_reflect_user_names_from_file(tmp_path):
    (tmp_path / "light.mxsl").write_text("class Light\n{\n    vec3 pos;\n}\n")
    reflection = mxslc.reflect_from_file(tmp_path / "light.mxsl")

    assert "Light" in reflection.type_names
