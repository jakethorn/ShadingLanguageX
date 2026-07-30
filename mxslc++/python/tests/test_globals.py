from pathlib import Path
import mxslc
import pytest

from data_utils import get_data_path, assert_matches_groundtruth


def test_globals_4_1():
    a = mxslc.create_variable(1.0)
    b = mxslc.create_variable(2.0)
    x = mxslc.create_variable([a, b])

    opts = mxslc.CompileOptions(globals = {"x": x})
    result = mxslc.compile_file_to_string(get_data_path("globals004.mxsl"), opts)
    assert_matches_groundtruth(result, "globals004.mtlx")


def test_globals_4_2():
    opts = mxslc.CompileOptions(globals = {"x": [1.0, 2.0]})
    result = mxslc.compile_file_to_string(get_data_path("globals004.mxsl"), opts)
    assert_matches_groundtruth(result, "globals004.mtlx")


def test_globals_4_3():
    a = mxslc.create_variable(1.0)

    opts = mxslc.CompileOptions(globals = {"x": [a, 2.0]})
    result = mxslc.compile_file_to_string(get_data_path("globals004.mxsl"), opts)
    assert_matches_groundtruth(result, "globals004.mtlx")


def test_globals_5():
    opts = mxslc.CompileOptions(globals = {"tint": 0.3, "albedo_path": "../brick.png"})
    result = mxslc.compile_file_to_string(get_data_path("globals005.mxsl"), opts)
    assert_matches_groundtruth(result, "globals005.mtlx")


try:
    import MaterialX as mx


    def test_globals_1_1():
        opts = mxslc.CompileOptions(globals = {
            "albedo_path": mxslc.create_variable(Path("../brick.png")),
            "tint": mxslc.create_variable(mx.Color3(1, 0, 0))
        })

        result = mxslc.compile_file_to_string(get_data_path("globals001.mxsl"), opts)
        assert_matches_groundtruth(result, "globals001.mtlx")


    def test_globals_1_2():
        opts = mxslc.CompileOptions(globals = {
            "albedo_path": Path("../brick.png"),
            "tint": mx.Color3(1, 0, 0)
        })

        result = mxslc.compile_file_to_string(get_data_path("globals001.mxsl"), opts)
        assert_matches_groundtruth(result, "globals001.mtlx")


    def test_globals_2():
        pos = mxslc.create_variable(mx.Vector3(0, 10, 0))
        dir = mx.Vector3(0, -1, 0)
        space = mxslc.create_variable("world")

        opts = mxslc.CompileOptions(globals = {"light0": [pos, dir, space]})
        result = mxslc.compile_file_to_string(get_data_path("globals002.mxsl"), opts)
        assert_matches_groundtruth(result, "globals002.mtlx")


    def test_globals_3():
        _0 = mxslc.create_variable(mx.Vector3(1, 3, 2))
        _1 = mxslc.create_variable(9.99)
        x = mxslc.create_variable([_0, _1])

        opts = mxslc.CompileOptions(globals = {"x": x})
        result = mxslc.compile_file_to_string(get_data_path("globals003.mxsl"), opts)
        assert_matches_groundtruth(result, "globals003.mtlx")


except ImportError:
    pass
