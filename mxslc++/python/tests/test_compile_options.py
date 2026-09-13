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


def test_compile_options_emit_source_hints():
    opts = mxslc.CompileOptions(emit_source_hints=True, reduce_graph=False)
    assert opts.emit_source_hints is True
    assert opts.decompile_hints is True

    # Positional argument in source
    pos_src = 'string greeting = constant("hello world");\n'
    pos_mtlx = mxslc.compile_string_to_string(pos_src, opts)
    assert 'mxsl:positional="true"' in pos_mtlx
    pos_decompiled = mxslc.decompile_string_to_string(pos_mtlx)
    assert pos_decompiled.strip() == pos_src.strip()

    # Named argument in source
    named_src = 'string greeting = constant(value = "hello world");\n'
    named_mtlx = mxslc.compile_string_to_string(named_src, opts)
    assert 'mxsl:named="true"' in named_mtlx
    named_decompiled = mxslc.decompile_string_to_string(named_mtlx)
    assert named_decompiled.strip() == named_src.strip()

    # Default (emit_source_hints = False) does not emit hints
    default_opts = mxslc.CompileOptions(reduce_graph=False)
    default_mtlx = mxslc.compile_string_to_string(pos_src, default_opts)
    assert 'mxsl:positional' not in default_mtlx
    assert 'mxsl:named' not in default_mtlx

