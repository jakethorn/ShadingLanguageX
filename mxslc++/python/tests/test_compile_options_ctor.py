import pytest
import mxslc
from pathlib import Path

def test_compile_options_ctor_defaults():
    options = mxslc.CompileOptions()

    assert options.output_file is None
    assert options.version == "1.39.5"
    assert options.func_name is None
    # cant test func_args because its write-only
    assert options.reduce_graph is True


def test_compile_options_ctor_with_kwargs():
    expected_path = Path("/tmp/out.mtlx")

    options = mxslc.CompileOptions(
        output_file=expected_path,
        version="1.38.10",
        func_name="my_custom_node",
        func_args=[1.0, (1.0, 0.5, 0.2), "diffuse.png"],
        reduce_graph=False
    )

    assert options.output_file == expected_path
    assert options.version == "1.38.10"
    assert options.func_name == "my_custom_node"
    # cant test func_args because its write-only
    assert options.reduce_graph is False


def test_compile_options_ctor_invalid_func_args():
    with pytest.raises(TypeError):
        mxslc.CompileOptions(verison="invalid_string_in_sequence")

    with pytest.raises(TypeError):
        mxslc.CompileOptions(func_args="invalid_string_in_sequence")

    with pytest.raises(TypeError):
        mxslc.CompileOptions(func_args=[{"value": 1.0}])
