import pytest
import mxslc

from data_utils import get_data_path, assert_matches_groundtruth


def test_macro_ctor():
    macro = mxslc.Macro("MY_MACRO")
    assert macro.name == "MY_MACRO"


def test_bad_macro_ctor():
    with pytest.raises(TypeError):
        macro = mxslc.Macro()


def test_macros_1_1():
    opts = mxslc.CompileOptions(macros = [])
    result = mxslc.compile_file_to_string(get_data_path("macros001.mxsl"), opts)
    assert_matches_groundtruth(result, "macros001_1.mtlx")


def test_macros_1_2():
    opts = mxslc.CompileOptions(macros = ["FOO"])
    result = mxslc.compile_file_to_string(get_data_path("macros001.mxsl"), opts)
    assert_matches_groundtruth(result, "macros001_2.mtlx")


def test_macros_2_1():
    opts = mxslc.CompileOptions(macros = [mxslc.Macro("VALUE1", "3.0")])
    result = mxslc.compile_file_to_string(get_data_path("macros002.mxsl"), opts)
    assert_matches_groundtruth(result, "macros002_1.mtlx")


def test_macros_2_2():
    opts = mxslc.CompileOptions(macros = [["VALUE1", "3.0"], ["VALUE2", "4.0"]])
    result = mxslc.compile_file_to_string(get_data_path("macros002.mxsl"), opts)
    assert_matches_groundtruth(result, "macros002_2.mtlx")


def test_macros_2_3():
    with pytest.raises(RuntimeError):
        opts = mxslc.CompileOptions(macros = [["VALUE2", "4.0"]])
        result = mxslc.compile_file_to_string(get_data_path("macros002.mxsl"), opts)
