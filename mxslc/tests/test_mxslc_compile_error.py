from pathlib import Path
from typing import Any

import pytest

import mxslc
from mxslc.CompileError import CompileError


@pytest.mark.parametrize("filename", [
    "bad_func_overload_1",
    "bad_func_overload_2",
    "amb_func_1",
    "amb_func_2",
    "amb_func_3",
    "bad_func_call_1",
    "bad_func_call_2",
    "bad_func_call_3",
    "delayed_var_decl_1",
    "missing_semi_1",
    "bad_data_type_1",
    "bad_data_type_2",
    "bad_data_type_3",
    "bad_data_type_4",
    "bad_data_type_5",
    "bad_template_1",
    "bad_template_2",
    "bad_template_3",
    "bad_arguments_1",
    "keyword_as_identifier",
    "inline_with_attribs",
    "out_param_1",
    "out_param_2",
    "const_1",
    "const_2",
])
def test_mxslc_compile_error(filename: str) -> None:
    mxsl_path = (Path(__file__).parent / "data" / "error" / filename).with_suffix(".mxsl")
    mtlx_path = (Path(__file__).parent / "data" / "error" / filename).with_suffix(".mtlx")

    with pytest.raises(CompileError):
        mxslc.compile_file(mxsl_path, validate=True)

    mtlx_path.unlink(missing_ok=True)


@pytest.mark.parametrize("filename, main_function, main_args", [
    ("bad_main_func_1", "my_function", []),
])
def test_mxslc_compile_error_main(filename: str, main_function: str | None, main_args: list) -> None:
    mxsl_path = (Path(__file__).parent / "data" / "error" / filename).with_suffix(".mxsl")
    mtlx_path = (Path(__file__).parent / "data" / "error" / filename).with_suffix(".mtlx")

    with pytest.raises(CompileError):
        mxslc.compile_file(mxsl_path, main_func=main_function, main_args=main_args, validate=True)

    mtlx_path.unlink(missing_ok=True)


@pytest.mark.parametrize("filename, globals_", [
    ("global_1", {}),
    ("global_1", {"x": 2.0}),
    ("global_2", {"x": 4.0}),
    ("global_3", {"x": 6.0}),
    ("global_4", {"x": 8.0}),
])
def test_mxslc_compile_error_globals(filename: str, globals_: dict[str, Any]) -> None:
    mxsl_path = (Path(__file__).parent / "data" / "error" / filename).with_suffix(".mxsl")
    mtlx_path = (Path(__file__).parent / "data" / "error" / filename).with_suffix(".mtlx")

    with pytest.raises(CompileError):
        mxslc.compile_file(mxsl_path, globals=globals_, validate=True)

    mtlx_path.unlink(missing_ok=True)
