import pytest
import mxslc
from pathlib import Path


def test_compile_options_ctor_defaults():
    options = mxslc.CompileOptions()

    assert options.output_file is None
    assert options.version == "1.39.5"
    assert len(options.macros) == 0
    assert len(options.search_directories) == 0
    assert len(options.includes) == 0
    assert len(options.libraries) == 0
    assert len(options.globals) == 0
    assert options.error_on_missing_globals == True
    assert options.error_on_unused_globals == True
    assert options.func_name is None
    assert len(options.func_args) == 0
    assert options.reduce_graph == True


def test_compile_options_ctor_with_kwargs():
    expected_path = Path("/tmp/out.mtlx")

    options = mxslc.CompileOptions(
        output_file=expected_path,
        version="1.38.10",
        macros=["HELLO", mxslc.Macro("WORLD", "UNIVERSE")],
        search_directories=[".", Path(".."), "/home/jaket/projects/MXSL/mxslc++"],
        includes=["math.mxsl"],
        libraries=["surfaces.mtlx"],
        globals={"base": 0.8, "is_srgb": True, "space": "model", "complex": [1, 2]},
        error_on_missing_globals=True,
        error_on_unused_globals=False,
        func_name="my_custom_node",
        func_args=[1, (1.0, 0.5, 0.2), "diffuse.png"],
        reduce_graph=False
    )

    assert options.output_file == expected_path

    assert options.version == "1.38.10"

    assert len(options.macros) == 2
    assert options.macros["HELLO"].name == "HELLO"
    assert options.macros["WORLD"].name == "WORLD"

    assert len(options.search_directories) == 3
    assert options.search_directories[0] == Path(".")
    assert options.search_directories[1] == Path("..")
    assert options.search_directories[2] == Path("/home/jaket/projects/MXSL/mxslc++")

    assert len(options.includes) == 1
    assert options.includes[0] == Path("math.mxsl")

    assert len(options.libraries) == 1
    assert options.libraries[0] == Path("surfaces.mtlx")

    assert len(options.globals) == 4
    assert options.globals["base"].value == pytest.approx(0.8)
    assert options.globals["is_srgb"].value is True
    assert options.globals["space"].value == "model"
    assert options.globals["complex"].value == [1, 2]

    assert options.error_on_missing_globals is True
    assert options.error_on_unused_globals is False

    assert options.func_name == "my_custom_node"

    assert len(options.func_args) == 3
    assert options.func_args[0].value == 1
    assert options.func_args[1].value == pytest.approx((1.0, 0.5, 0.2))
    assert options.func_args[2].value == "diffuse.png"

    assert options.reduce_graph is False


def test_compile_options_properties():
    expected_path = Path("/tmp/out.mtlx")

    options = mxslc.CompileOptions()
    options.output_file = expected_path
    options.version = "1.38.10"
    options.macros = ["HELLO", mxslc.Macro("WORLD", "UNIVERSE")]
    options.search_directories = [".", Path(".."), "/home/jaket/projects/MXSL/mxslc++"]
    options.includes = ["math.mxsl"]
    options.libraries = ["surfaces.mtlx"]
    options.globals = {"base": 0.8, "is_srgb": True, "space": "model", "complex": [1, 2]}
    options.error_on_missing_globals = True
    options.error_on_unused_globals = False
    options.func_name = "my_custom_node"
    options.func_args = [1, (1.0, 0.5, 0.2), "diffuse.png"]
    options.reduce_graph = False

    assert options.output_file == expected_path

    assert options.version == "1.38.10"

    assert len(options.macros) == 2
    assert options.macros["HELLO"].name == "HELLO"
    assert options.macros["WORLD"].name == "WORLD"

    assert len(options.search_directories) == 3
    assert options.search_directories[0] == Path(".")
    assert options.search_directories[1] == Path("..")
    assert options.search_directories[2] == Path("/home/jaket/projects/MXSL/mxslc++")

    assert len(options.includes) == 1
    assert options.includes[0] == Path("math.mxsl")

    assert len(options.libraries) == 1
    assert options.libraries[0] == Path("surfaces.mtlx")

    assert len(options.globals) == 4
    assert options.globals["base"].value == pytest.approx(0.8)
    assert options.globals["is_srgb"].value is True
    assert options.globals["space"].value == "model"
    assert options.globals["complex"].value == [1, 2]

    assert options.error_on_missing_globals is True
    assert options.error_on_unused_globals is False

    assert options.func_name == "my_custom_node"

    assert len(options.func_args) == 3
    assert options.func_args[0].value == 1
    assert options.func_args[1].value == pytest.approx((1.0, 0.5, 0.2))
    assert options.func_args[2].value == "diffuse.png"

    assert options.reduce_graph is False


def test_compile_options_ctor_invalid_args():
    with pytest.raises(TypeError):
        mxslc.CompileOptions(verison="1.39.5")

    with pytest.raises(TypeError):
        mxslc.CompileOptions(macros="bad macro")

    with pytest.raises(TypeError):
        mxslc.CompileOptions(globals="bad global")

    with pytest.raises(TypeError):
        mxslc.CompileOptions(globals={1: "a", 2: "b"})

    with pytest.raises(TypeError):
        mxslc.CompileOptions(func_args="bad argument")

    with pytest.raises(TypeError):
        mxslc.CompileOptions(func_args={"value": 1.0})

    with pytest.raises(TypeError):
        mxslc.CompileOptions(func_args=[{"value": 1.0}])
