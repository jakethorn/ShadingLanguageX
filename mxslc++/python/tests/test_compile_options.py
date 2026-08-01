import mxslc
import pytest

from groundtruth import STANDARD_OUTPUT, REDUCED_OUTPUT
from data_utils import get_data_path, assert_matches_groundtruth


def test_compile_options_reduce_graph_is_false():
    options = mxslc.CompileOptions()
    options.reduce_graph = False

    result = mxslc.compile_string_to_string("float f = 1.0 + 1.0;", options)
    assert result == STANDARD_OUTPUT


def test_compile_options_reduce_graph_is_true():
    options = mxslc.CompileOptions()
    options.reduce_graph = True

    result = mxslc.compile_string_to_string("float f = 1.0 + 1.0;", options)
    assert result == REDUCED_OUTPUT


def test_compile_options_with_non_default_version():
    options = mxslc.CompileOptions(version="1.39.4")

    result = mxslc.compile_string_to_string("float f = 1.0 + 1.0;", options)
    assert result == REDUCED_OUTPUT


def test_compile_options_with_old_version():
    options = mxslc.CompileOptions(version="1.38.10")

    result = mxslc.compile_file_to_string(get_data_path("version1_38_10.mxsl"), options)
    assert_matches_groundtruth(result, "version1_38_10.mtlx")


def test_compile_options_with_invalid_version():
    options = mxslc.CompileOptions(version="bad_version_string")

    with pytest.raises(RuntimeError, match=f"Invalid MaterialX version: {options.version}"):
        mxslc.compile_string_to_string("float f = 1.0 + 1.0;", options)
