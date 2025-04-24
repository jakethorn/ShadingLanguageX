from pathlib import Path

import pytest

import mxslc

_overwrite_all_expected = False


@pytest.mark.parametrize("filename, overwrite_expected", [
    ("interiormapping_room", False),
    ("interiormapping_words", False),
    ("redbrick", False),
    ("shaderart", False),
    ("condensation", False),
    ("binary_expressions", False),
    ("for_loops_1", False),
    ("for_loops_2", False),
    ("for_loops_3", False),
    ("for_loops_4", False),
    ("for_loops_5", False),
    ("named_arguments_1", False),
    ("named_arguments_2", False),
    ("node_constructors_1", False),
    ("test_001", False),
    ("test_002", False),
    ("test_003", False),
    ("test_004", False),
    ("test_005", False),
    ("test_006", False),
    ("test_007", False),
    ("test_008", False),
    ("test_009", False),
    ("test_010", False),
    ("includes_1", False),
    ("properties_1", False),
    ("mountain", False),
    ("expr_stmt_1", False),
    ("directives/directives_1", False),
    ("directives/directives_2", False),
    ("directives/directives_3", False),
    ("directives/directives_4", False),
])
def test_mxslc(filename: str, overwrite_expected: bool) -> None:
    mxsl_path     = (Path(__file__).parent / "data" / "mxsl" / filename).with_suffix(".mxsl")
    actual_path   = (Path(__file__).parent / "data" / "mxsl" / filename).with_suffix(".mtlx")
    expected_path = (Path(__file__).parent / "data" / "mtlx" / filename).with_suffix(".mtlx")

    if overwrite_expected or _overwrite_all_expected:
        actual_path = expected_path

    mxslc.compile_file(mxsl_path, actual_path)

    with open(actual_path, "r") as f:
        actual = f.read()

    with open(expected_path, "r") as f:
        expected = f.read()

    if not (overwrite_expected or _overwrite_all_expected):
        actual_path.unlink()

    assert actual.replace("\\", "/") == expected.replace("\\", "/")
