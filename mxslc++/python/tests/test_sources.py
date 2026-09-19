import mxslc
import pytest

ROOT = '#include "values.mxsl"\nfloat z = value + 1.0;'
VALUES = "float value = 12;"


def test_include_resolves_from_an_in_memory_source():
    options = mxslc.CompileOptions()
    # Graph reduction would fold the constant float away, leaving nothing to
    # assert on, so keep the nodes.
    options.reduce_graph = False
    options.add_source("values.mxsl", VALUES)

    result = mxslc.compile_string_to_string(ROOT, options)

    # `value` only exists in the in-memory source, so its constant appearing in
    # the graph proves the contents were used, not just that some file of that
    # name was resolved.
    assert 'name="z"' in result
    assert 'value="12"' in result


def test_sources_can_be_set_from_a_dict():
    options = mxslc.CompileOptions(sources={"values.mxsl": VALUES})

    result = mxslc.compile_string_to_string(ROOT, options)

    assert "<materialx" in result


def test_sources_are_readable():
    options = mxslc.CompileOptions()
    options.add_source("values.mxsl", VALUES)

    assert options.sources == {"values.mxsl": VALUES}
    assert options.has_source("values.mxsl")


def test_clearing_sources_removes_them():
    options = mxslc.CompileOptions()
    options.add_source("values.mxsl", VALUES)

    options.clear_sources()

    assert not options.has_source("values.mxsl")
    with pytest.raises(RuntimeError, match="values.mxsl"):
        mxslc.compile_string_to_string(ROOT, options)


def test_nested_include_resolves_relative_to_the_including_source():
    options = mxslc.CompileOptions()
    options.reduce_graph = False
    # values.mxsl includes "base.mxsl", which must resolve to sub/base.mxsl
    # rather than to a file at the top of the overlay.
    options.add_source("sub/values.mxsl", '#include "base.mxsl"\nfloat value = base + base;')
    options.add_source("sub/base.mxsl", "float base = 6;")

    result = mxslc.compile_string_to_string(
        '#include "sub/values.mxsl"\nfloat z = value + 1.0;', options
    )

    assert 'name="z"' in result


def test_unsupplied_include_raises():
    options = mxslc.CompileOptions()

    with pytest.raises(RuntimeError, match="values.mxsl"):
        mxslc.compile_string_to_string(ROOT, options)
