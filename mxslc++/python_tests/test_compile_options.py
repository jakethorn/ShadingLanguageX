import mxslc
import pytest

from groundtruth import STANDARD_OUTPUT, REDUCED_OUTPUT


def test_compile_options_output_file_defaults_to_none():
    options = mxslc.CompileOptions()

    assert options.output_file is None
    assert options.version == "1.39.5"
    assert options.reduce_graph == True


def test_compile_options_reduce_graph_is_false():
    options = mxslc.CompileOptions()
    options.reduce_graph = False

    result = mxslc.compile_string_to_string("float f = 1.0 + 1.0;", options)

    assert isinstance(result, str)
    assert result == STANDARD_OUTPUT


def test_compile_options_reduce_graph_is_true_by_default():
    options = mxslc.CompileOptions()

    result = mxslc.compile_string_to_string("float f = 1.0 + 1.0;", options)

    assert isinstance(result, str)
    assert result == REDUCED_OUTPUT


def test_compile_options_with_non_default_version():
    options = mxslc.CompileOptions()
    options.version = "1.39.4"

    result = mxslc.compile_string_to_string("float f = 1.0 + 1.0;", options)

    assert isinstance(result, str)
    assert result == REDUCED_OUTPUT


def test_compile_options_with_invalid_version():
    options = mxslc.CompileOptions()
    options.version = "bad_version_string"

    with pytest.raises(RuntimeError, match=f"MaterialX version {options.version} libraries could not be found"):
        mxslc.compile_string_to_string("float f = 1.0 + 1.0;", options)
