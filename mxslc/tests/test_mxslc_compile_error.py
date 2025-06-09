from pathlib import Path

import pytest

import mxslc
from mxslc.CompileError import CompileError


@pytest.mark.parametrize("filename, main_function, main_args", [
    ("bad_main_func_1", "my_function", []),
    ("bad_func_overload_1", None, []),
    ("bad_func_overload_2", None, []),
    ("amb_func_1", None, []),
    ("amb_func_2", None, []),
    ("amb_func_3", None, []),
    ("bad_func_call_1", None, []),
    ("delayed_var_decl_1", None, []),
    ("missing_semi_1", None, []),
    ("bad_data_type_1", None, []),
    ("bad_data_type_2", None, []),
    ("bad_data_type_3", None, []),
])
def test_mxslc_compile_error(filename: str, main_function: str | None, main_args: list) -> None:
    mxsl_path = (Path(__file__).parent / "data" / "error" / filename).with_suffix(".mxsl")
    mtlx_path = (Path(__file__).parent / "data" / "error" / filename).with_suffix(".mtlx")

    with pytest.raises(CompileError):
        mxslc.compile_file(mxsl_path, main_func=main_function, main_args=main_args)

    mtlx_path.unlink(missing_ok=True)
