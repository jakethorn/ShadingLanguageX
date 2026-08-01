import pytest
import mxslc


def test_create_variable():
    var = mxslc.create_variable(1)
    assert hasattr(var, "value")


def test_create_variables_1():
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
