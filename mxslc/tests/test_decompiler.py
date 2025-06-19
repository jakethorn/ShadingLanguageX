from pathlib import Path
from warnings import warn

import pytest

import mxslc

_overwrite_all_expected = False


@pytest.mark.parametrize("filename, overwrite_expected", [
    ("decompiler/disintegrate", False),
    ("decompiler/gold", False),
    ("decompiler/interiormapping", False),
    ("decompiler/mountain", False),
    ("decompiler/rain", False),
    ("decompiler/redbrick", False),
    ("decompiler/shaderart", False),
    ("decompiler/squares", False),
    ("decompiler/toon", False),
    ("decompiler/waves", False),
])
def test_decompiler(filename: str, overwrite_expected: bool) -> None:
    mtlx_path     = (Path(__file__).parent / "data" / "mtlx" / filename).with_suffix(".mtlx")
    actual_path   = (Path(__file__).parent / "data" / "mtlx" / filename).with_suffix(".mxsl")
    expected_path = (Path(__file__).parent / "data" / "mxsl" / filename).with_suffix(".mxsl")

    if overwrite_expected or _overwrite_all_expected:
        warn(f"Expected data for {filename} is being overwritten.")
        actual_path = expected_path

    mxslc.decompile_file(mtlx_path, actual_path)

    with open(actual_path, "r") as f:
        actual = f.read()

    with open(expected_path, "r") as f:
        expected = f.read()

    if not (overwrite_expected or _overwrite_all_expected):
        actual_path.unlink()

    assert actual.replace("\\", "/") == expected.replace("\\", "/")

    # 2nd test - compile the decompiled shader
    mxsl_path = expected_path
    actual_path = (Path(__file__).parent / "data" / "mxsl" / filename).with_suffix(".mtlx")
    mxslc.compile_file(mxsl_path, actual_path)
    actual_path.unlink()
