from pathlib import Path
from warnings import warn

import pytest

import mxslc

_overwrite_all_expected = False


@pytest.mark.parametrize("filename, overwrite_expected", [
    ("interiormapping_room", False),
    ("interiormapping_words", False),
    ("redbrick", False),
    ("shaderart", False),
    ("condensation", False),
    ("binary_expressions_1", False),
    ("for_loops/for_loops_1", False),
    ("for_loops/for_loops_2", False),
    ("for_loops/for_loops_3", False),
    ("for_loops/for_loops_4", False),
    ("for_loops/for_loops_5", False),
    ("named_arguments_1", False),
    ("named_arguments_2", False),
    ("node_constructors_1", False),
    ("simple/test_001", False),
    ("simple/test_002", False),
    ("simple/test_003", False),
    ("simple/test_004", False),
    ("simple/test_005", False),
    ("simple/test_006", False),
    ("simple/test_007", False),
    ("simple/test_008", False),
    ("simple/test_009", False),
    ("simple/test_010", False),
    ("includes_1", False),
    ("properties_1", False),
    ("mountain", False),
    ("expr_stmt_1", False),
    ("directives/directives_1", False),
    ("directives/directives_2", False),
    ("directives/directives_3", False),
    ("directives/directives_4", False),
    ("directives/directives_5", False),
    ("tern_rel_expr_1", False),
    ("func_overloads/func_overloads_1", False),
    ("func_overloads/func_overloads_2", False),
    ("func_overloads/func_overloads_3", False),
    ("func_overloads/func_overloads_4", False),
    ("func_overloads/func_overloads_5", False),
    ("func_overloads/func_overloads_6", False),
    ("templates/templates_1", False),
    ("templates/templates_2", False),
    ("templates/templates_3", False),
    ("templates/templates_4", False),
    ("templates/templates_5", False),
    ("templates/templates_6", False),
    ("default_values_1", False),
    ("default_values_2", False),
    ("empty_file", False),
    ("node_defs/multioutput_1", False),
    ("node_defs/multioutput_2", False),
    ("node_defs/multioutput_3", False),
    ("node_defs/multioutput_4", False),
    ("node_defs/multioutput_5", False),
    ("node_defs/multioutput_6", False),
    ("node_defs/localvars_1", False),
    ("node_defs/localvars_2", False),
    ("node_defs/complex_func_1", False),
    ("node_defs/complex_func_2", False),
    ("node_defs/complex_func_3", False),
    ("node_defs/complex_func_4", False),
    ("auto_vars_1", False),
    ("auto_vars_2", False),
    ("auto_vars_3", False),
    ("bsdf_1", False),
    ("bsdf_2", False),
    ("attributes/attributes_1", False),
    ("attributes/attributes_2", False),
    ("attributes/attributes_3", False),
    ("attributes/attributes_4", False),
    ("inline/inline_test_1", False),
    ("inline/inline_test_2", False),
    ("inline/inline_test_3", False),
    ("inline/inline_test_4", False),
    ("inline/inline_test_5", False),
    ("inline/inline_test_6", False),
    ("inline/inline_test_7", False),
    ("inline/inline_test_8", False),
    ("inline/inline_test_9", False),
    ("inline/inline_template_test", False),
])
def test_mxslc(filename: str, overwrite_expected: bool) -> None:
    mxsl_path     = (Path(__file__).parent / "data" / "mxsl" / filename).with_suffix(".mxsl")
    actual_path   = (Path(__file__).parent / "data" / "mxsl" / filename).with_suffix(".mtlx")
    expected_path = (Path(__file__).parent / "data" / "mtlx" / filename).with_suffix(".mtlx")

    if overwrite_expected or _overwrite_all_expected:
        warn(f"Expected data for {filename} is being overwritten.")
        actual_path = expected_path

    mxslc.compile_file(mxsl_path, actual_path)

    with open(actual_path, "r") as f:
        actual = f.read()

    with open(expected_path, "r") as f:
        expected = f.read()

    if not (overwrite_expected or _overwrite_all_expected):
        actual_path.unlink()

    assert actual.replace("\\", "/") == expected.replace("\\", "/")
