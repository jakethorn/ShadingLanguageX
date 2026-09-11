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

        mtlx -> mxsl -> mtlx

    and requires every stage to pass (no exception).
    """
    mtlx = get_data("separate_combine.mtlx")

    # mtlx -> mxsl: separate3 outputs must be indexed, never member-named.
    mxsl1 = mxslc.decompile_string_to_string(mtlx)
    assert_matches_groundtruth(mxsl1, "separate_combine.mxsl")

    # mxsl -> mtlx
    mtlx2 = mxslc.compile_string_to_string(mxsl1)
    assert_matches_groundtruth(mtlx2, "separate_combine.mtlx")


def test_decompile_inline_ng_output_ref_roundtrip():
    """A nodegraph whose outputs are inline referenced via round-trip test:

        mtlx -> mxsl -> mtlx

    to ensure consistency of output declarations and references.
    """
    mtlx = get_data("decompile_inline_ng_output_ref.mtlx")

    # mtlx -> mxsl: nodegraph output references use unprefixed field names.
    mxsl = mxslc.decompile_string_to_string(mtlx)
    assert_matches_groundtruth(mxsl, "decompile_inline_ng_output_ref.mxsl")

    # mxsl -> mtlx
    mtlx2 = mxslc.compile_string_to_string(mxsl)
    assert_matches_groundtruth(mtlx2, "decompile_inline_ng_output_ref.mtlx")


def test_compile_multioutput_reference():
    result = mxslc.compile_file_to_string(get_data_path("entry007.mxsl"))
    # Remove output="outcolor" from result to match original default output
    result = result.replace('output="outcolor" ', "")

    assert_matches_groundtruth(result, "entry007.mtlx")
