import pytest
import mxslc


def test_create_variable():
    var = mxslc.create_variable(1)
    assert hasattr(var, "value")


def test_create_variables_1():
    mxslc.compile_string_to_string("")
    var = mxslc.create_variable(1)
    assert var.value == 1
    var = mxslc.create_variable(3.14)
    assert var.value == pytest.approx(3.14)
    var = mxslc.create_variable([1.0, 2.0, 3.0])
    assert var.value == pytest.approx([1.0, 2.0, 3.0])
    var = mxslc.create_variable([1, "hello", "world", True])
    assert var.value == [1, "hello", "world", True]

try:
    import MaterialX as mx

    def test_create_variables_2():
        var = mxslc.create_variable(mx.Color3(1, 0, 0))
        assert isinstance(var.value, mx.Color3)
        assert var.value == mx.Color3(1, 0, 0)

except ImportError:
    pass


def test_mutable_variable_roundtrip():
    src = """mutable float a = sin(1);
a = a + 2;
float x = a + 10;
a = a * 4;
float b = a * 3;
"""
    opts = mxslc.CompileOptions(emit_source_hints=True, reduce_graph=False)
    mtlx = mxslc.compile_string_to_string(src, opts)
    decompiled = mxslc.decompile_string_to_string(mtlx).strip()
    assert decompiled == src.strip()


def test_mutable_literal_reassign_roundtrip():
    src = """mutable int i = 0;
i = i + 2;
constant(i);
"""
    opts = mxslc.CompileOptions(emit_source_hints=True, reduce_graph=False)
    mtlx = mxslc.compile_string_to_string(src, opts)
    decompiled = mxslc.decompile_string_to_string(mtlx).strip()
    assert decompiled == src.strip()


def test_const_variable_roundtrip():
    src = """const vec2 uv = position().xy;
float x = uv.x;
"""
    opts = mxslc.CompileOptions(emit_source_hints=True, reduce_graph=False)
    mtlx = mxslc.compile_string_to_string(src, opts)
    decompiled = mxslc.decompile_string_to_string(mtlx).strip()
    assert decompiled == src.strip()


def test_const_literal_roundtrip():
    src = """const float x = 42;
const float y = 3.14;
float z = x + y;
"""
    opts = mxslc.CompileOptions(emit_source_hints=True, reduce_graph=False)
    mtlx = mxslc.compile_string_to_string(src, opts)
    decompiled = mxslc.decompile_string_to_string(mtlx).strip()
    assert decompiled == src.strip()


def test_null_argument_roundtrip():
    src = """surfacematerial(standard_surface(base_color = color3{0.82, 0.796, 0.761}, metalness = 0, specular_roughness = 1), null, displacement(0.5));
"""
    opts = mxslc.CompileOptions(emit_source_hints=True, reduce_graph=False)
    mtlx = mxslc.compile_string_to_string(src, opts)
    decompiled = mxslc.decompile_string_to_string(mtlx).strip()
    assert decompiled == src.strip()
    mtlx2 = mxslc.compile_string_to_string(decompiled, opts)
    assert mtlx == mtlx2


