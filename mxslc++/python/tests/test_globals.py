from pathlib import Path
import mxslc
import pytest

from data_utils import get_data_path, get_data, write_data


def test_globals_3():
    _0 = mxslc.Variable((1, 3, 2))
    _1 = mxslc.Variable(9.99)
    x = mxslc.Variable("x", [_0, _1])

    opts = mxslc.CompileOptions(globals = [x])
    result = mxslc.compile_file_to_string(get_data_path("globals003.mxsl"), opts)
    write_data("globals003.mtlx", result)
    assert result == get_data("globals003.mtlx")


def test_globals_4_1():
    a = mxslc.Variable("a", 1.0)
    b = mxslc.Variable("b", 2.0)
    x = mxslc.Variable("x", [a, b])

    opts = mxslc.CompileOptions(globals = [x])
    result = mxslc.compile_file_to_string(get_data_path("globals004.mxsl"), opts)
    write_data("globals004.mtlx", result)
    assert result == get_data("globals004.mtlx")


def test_globals_4_2():
    a = mxslc.Variable(1.0)
    b = mxslc.Variable(2.0)
    x = mxslc.Variable("x", [a, b])

    opts = mxslc.CompileOptions(globals = [x])
    result = mxslc.compile_file_to_string(get_data_path("globals004.mxsl"), opts)
    write_data("globals004.mtlx", result)
    assert result == get_data("globals004.mtlx")


try:
    import MaterialX as mx

    def test_globals_1():
        opts = mxslc.CompileOptions(globals = [
            mxslc.Variable("albedo_path", Path("../brick.png")),
            mxslc.Variable("tint", mx.Color3(1, 0, 0))
        ])

        result = mxslc.compile_file_to_string(get_data_path("globals001.mxsl"), opts)
        write_data("globals001.mtlx", result)
        assert result == get_data("globals001.mtlx")

    def test_globals_2():
        pos = mxslc.Variable("pos", mx.Vector3(0, 10, 0))
        dir = mxslc.Variable("dir", mx.Vector3(0, -1, 0))
        space = mxslc.Variable("space", "world")
        light0 = mxslc.Variable("light0", [pos, dir, space])

        opts = mxslc.CompileOptions(globals = [light0])
        result = mxslc.compile_file_to_string(get_data_path("globals002.mxsl"), opts)
        write_data("globals002.mtlx", result)
        assert result == get_data("globals002.mtlx")

except ImportError:
    pass
