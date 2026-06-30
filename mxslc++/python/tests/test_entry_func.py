import mxslc
import pytest

from data_utils import get_data_path, get_data, write_data


def test_entry_func_with_no_name_or_args():
    result = mxslc.compile_file_to_string(get_data_path("entry004.mxsl"))

    assert isinstance(result, str)
    write_data("empty.mtlx", result)
    assert result == get_data("empty.mtlx")


def test_entry_func_with_correct_name_and_args():
    opts = mxslc.CompileOptions(func_name="foo", func_args=[6.66, (1, 0, 0), "world"])
    result = mxslc.compile_file_to_string(get_data_path("entry004.mxsl"), opts)

    assert isinstance(result, str)
    write_data("entry004.mtlx", result)
    assert result == get_data("entry004.mtlx")


def test_entry_func_with_incorrect_name_and_args():
    with pytest.raises(RuntimeError):
        opts = mxslc.CompileOptions(func_name="foo", func_args=["hello", "world", 3.14])
        mxslc.compile_file_to_string(get_data_path("entry004.mxsl"), opts)

try:
    import MaterialX as mx

    def test_entry_func_with_correct_name_and_args_mx():
        opts = mxslc.CompileOptions(func_name="foo", func_args=[6.66, mx.Color3(0, 0, 1), "world"])
        result = mxslc.compile_file_to_string(get_data_path("entry005.mxsl"), opts)

        assert isinstance(result, str)
        write_data("entry005.mtlx", result)
        assert result == get_data("entry005.mtlx")

except ImportError:
    pass
