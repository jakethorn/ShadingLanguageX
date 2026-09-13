import mxslc


def test_range_loop_to_roundtrip():
    src = """mutable int i = 0;
for (int j from 0 to 9)
{
	i = i + 2;
}
constant(i);
"""
    opts = mxslc.CompileOptions(emit_source_hints=True, reduce_graph=False)
    mtlx1 = mxslc.compile_string_to_string(src, opts)
    decompiled = mxslc.decompile_string_to_string(mtlx1)
    assert decompiled.strip() == src.strip()
    mtlx2 = mxslc.compile_string_to_string(decompiled, opts)
    assert mtlx1 == mtlx2


def test_range_loop_colon_roundtrip():
    src = """mutable int i = 0;
for (int j from 0:9)
{
	i = i + 1;
}
constant(i);
"""
    opts = mxslc.CompileOptions(emit_source_hints=True, reduce_graph=False)
    mtlx1 = mxslc.compile_string_to_string(src, opts)
    decompiled = mxslc.decompile_string_to_string(mtlx1)
    assert decompiled.strip() == src.strip()
    mtlx2 = mxslc.compile_string_to_string(decompiled, opts)
    assert mtlx1 == mtlx2


def test_range_loop_step_roundtrip():
    src = """mutable int i = 0;
for (int j from 0:2:9)
{
	i = i + 1;
}
constant(i);
"""
    opts = mxslc.CompileOptions(emit_source_hints=True, reduce_graph=False)
    mtlx1 = mxslc.compile_string_to_string(src, opts)
    decompiled = mxslc.decompile_string_to_string(mtlx1)
    assert decompiled.strip() == src.strip()
    mtlx2 = mxslc.compile_string_to_string(decompiled, opts)
    assert mtlx1 == mtlx2


def test_collection_loop_roundtrip():
    src = """for (string s from {"hello", "world"})
{
	constant(s);
}
"""
    opts = mxslc.CompileOptions(emit_source_hints=True, reduce_graph=False)
    mtlx1 = mxslc.compile_string_to_string(src, opts)
    decompiled = mxslc.decompile_string_to_string(mtlx1)
    assert decompiled.strip() == src.strip()
    mtlx2 = mxslc.compile_string_to_string(decompiled, opts)
    assert mtlx1 == mtlx2


def test_loop_accumulator_roundtrip():
    src = """mutable int sum = 0;
for (int j from 0 to 4)
{
	int k = j * 2;
	sum = sum + k;
}
constant(sum);
"""
    opts = mxslc.CompileOptions(emit_source_hints=True, reduce_graph=False)
    mtlx1 = mxslc.compile_string_to_string(src, opts)
    decompiled = mxslc.decompile_string_to_string(mtlx1)
    assert decompiled.strip() == src.strip()
    mtlx2 = mxslc.compile_string_to_string(decompiled, opts)
    assert mtlx1 == mtlx2
