import mxslc
import pytest

from data_utils import get_data_path, get_data, assert_matches_groundtruth


def test_entry_func_with_no_name_or_args():
    result = mxslc.compile_file_to_string(get_data_path("entry004.mxsl"))
    assert_matches_groundtruth(result, "empty.mtlx")


def test_entry_func_with_correct_name_and_args():
    opts = mxslc.CompileOptions(func_name="foo", func_args=[6.66, (1.0, 0.0, 0.0), "world"])
    result = mxslc.compile_file_to_string(get_data_path("entry004.mxsl"), opts)
    assert_matches_groundtruth(result, "entry004.mtlx")


def test_entry_func_with_complex_args():
    opts = mxslc.CompileOptions(func_name="foo", func_args=[6.66, [0.5, "world"]])
    result = mxslc.compile_file_to_string(get_data_path("entry006.mxsl"), opts)
    assert_matches_groundtruth(result, "entry006.mtlx")


def test_entry_func_with_incorrect_name():
    with pytest.raises(RuntimeError):
        opts = mxslc.CompileOptions(func_name="boo", func_args=[6.66, (1.0, 0.0, 0.0), "world"])
        mxslc.compile_file_to_string(get_data_path("entry004.mxsl"), opts)


def test_entry_func_with_incorrect_args():
    with pytest.raises(RuntimeError):
        opts = mxslc.CompileOptions(func_name="foo", func_args=["hello", "world", 3.14])
        mxslc.compile_file_to_string(get_data_path("entry004.mxsl"), opts)

try:
    import MaterialX as mx

    def test_entry_func_with_correct_name_and_args_mx():
        opts = mxslc.CompileOptions(func_name="foo", func_args=[6.66, mx.Color3(0, 0, 1), "world"])
        result = mxslc.compile_file_to_string(get_data_path("entry005.mxsl"), opts)
        assert_matches_groundtruth(result, "entry005.mtlx")

except ImportError:
    pass


def test_decompile_multioutput_reference():
    result = mxslc.decompile_file_to_string(get_data_path("entry007.mtlx"))
    assert result == get_data("entry007.mxsl")


def test_compile_multioutput_reference():
    result = mxslc.compile_file_to_string(get_data_path("entry007.mxsl"))
    # Remove output="outcolor" from result to match original default output
    result = result.replace('output="outcolor" ', "")

    assert_matches_groundtruth(result, "entry007.mtlx")
