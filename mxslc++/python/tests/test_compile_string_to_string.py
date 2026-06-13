import mxslc
import pytest

from groundtruth import RANDOMFLOAT_OUTPUT


def test_compile_string_to_string():
    result = mxslc.compile_string_to_string("float f = randomfloat() + 1.0;")

    assert isinstance(result, str)
    assert result == RANDOMFLOAT_OUTPUT


def test_compile_string_to_string_with_invalid_code():
    with pytest.raises(RuntimeError, match="line 1: Invalid statement"):
        mxslc.compile_string_to_string("float 1.0")
