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


def test_inline_separate_roundtrip():
    """An mtlx that inlines a separate3 output (outx/outz feeding a combine3) must
    decompile to compilable mxsl. A separate* node returns an anonymous array, so
    its outputs must be addressed by index rather than `.outx`/`.outz` member
    access. Round-trips the document as

        mtlx -> mxsl -> mtlx -> mxsl -> mtlx

    and requires every stage to pass (no exception).
    """
    mtlx0 = get_data("separate_combine.mtlx")

    # mtlx -> mxsl
    mxsl1 = mxslc.decompile_string_to_string(mtlx0)
    # mxsl -> mtlx
    mtlx2 = mxslc.compile_string_to_string(mxsl1)
    # mtlx -> mxsl
    mxsl3 = mxslc.decompile_string_to_string(mtlx2)
    # Final mxsl -> mtlx must simply compile (pass), i.e. the decompiled mxsl is valid.
    mxslc.compile_string_to_string(mxsl3)


def test_compile_multioutput_reference():
    result = mxslc.compile_file_to_string(get_data_path("entry007.mxsl"))
    # Remove output="outcolor" from result to match original default output
    result = result.replace('output="outcolor" ', "")

    assert_matches_groundtruth(result, "entry007.mtlx")
