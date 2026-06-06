import mxslc
import pytest


RANDOMFLOAT_OUTPUT = """<?xml version="1.0"?>
<materialx version="1.39">
  <randomfloat name="node1" type="float" />
  <add name="node2" type="float">
    <input name="in1" type="float" nodename="node1" />
    <input name="in2" type="float" value="1" />
  </add>
</materialx>
"""


def test_compile_string_to_string_randomfloat():
    result = mxslc.compile_string_to_string("float f = randomfloat() + 1.0;")

    assert isinstance(result, str)
    assert result == RANDOMFLOAT_OUTPUT


def test_compile_string_to_string_with_invalid_code():
    with pytest.raises(RuntimeError, match="line 1: Invalid statement"):
        mxslc.compile_string_to_string("float 1.0")
