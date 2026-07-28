import mxslc
import pytest


STRING_CONSTANT_MTLX = """<?xml version="1.0"?>
<materialx version="1.39">
  <constant name="out" type="string">
    <input name="in" type="string" value="hello world" />
  </constant>
</materialx>
"""

STRING_CONSTANT_EXPECTED = 'string out = constant(in = "hello world");\n'

FILENAME_IMAGE_MTLX = """<?xml version="1.0"?>
<materialx version="1.39">
  <image name="out" type="color3">
    <input name="file" type="filename" value="textures/albedo.tif" />
  </image>
</materialx>
"""

FILENAME_IMAGE_EXPECTED = 'color3 out = image(file = "textures/albedo.tif");\n'

EMPTY_STRING_MTLX = """<?xml version="1.0"?>
<materialx version="1.39">
  <constant name="out" type="string">
    <input name="in" type="string" value="" />
  </constant>
</materialx>
"""

EMPTY_STRING_EXPECTED = 'string out = constant(in = "");\n'

def test_decompile_string_constant():
    """Decompiling a string constant should preserve quotes around string values."""
    result = mxslc.decompile_string_to_string(STRING_CONSTANT_MTLX)
    assert isinstance(result, str)
    assert result == STRING_CONSTANT_EXPECTED, f"Expected:\n{STRING_CONSTANT_EXPECTED!r}\nGot:\n{result!r}"


def test_decompile_filename_input():
    """Decompiling a filename input should preserve quotes around filename values."""
    result = mxslc.decompile_string_to_string(FILENAME_IMAGE_MTLX)
    assert isinstance(result, str)
    assert result == FILENAME_IMAGE_EXPECTED, f"Expected:\n{FILENAME_IMAGE_EXPECTED!r}\nGot:\n{result!r}"


def test_decompile_empty_string():
    """Decompiling an empty string value should still preserve quotes."""
    result = mxslc.decompile_string_to_string(EMPTY_STRING_MTLX)
    assert isinstance(result, str)
    assert result == EMPTY_STRING_EXPECTED, f"Expected:\n{EMPTY_STRING_EXPECTED!r}\nGot:\n{result!r}"


def test_decompile_quotes_on_inputs():
    """Multiple inputs with string/filename values all get proper quotes."""
    mtlx = """<?xml version="1.0"?>
<materialx version="1.39">
  <ifequal name="out" type="string">
    <input name="value1" type="string" value="left" />
    <input name="value2" type="string" value="right" />
    <input name="in1" type="string" value="match" />
    <input name="in2" type="string" value="no_match" />
  </ifequal>
</materialx>
"""
    result = mxslc.decompile_string_to_string(mtlx)
    # All four string values should be quoted in the output
    assert '"left"' in result
    assert '"right"' in result
    assert '"match"' in result
    assert '"no_match"' in result
