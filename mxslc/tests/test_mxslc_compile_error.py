from pathlib import Path

import pytest

import mxslc
from mxslc.CompileError import CompileError


@pytest.mark.parametrize("filename, main_function, main_args", [
    ("bad_main_function_1", "my_function", []),
    ("bad_func_overload_1", None, []),
    ("bad_func_overload_2", None, []),
])
def test_mxslc_compile_error(filename: str, main_function: str | None, main_args: list) -> None:
    mxsl_path = (Path(__file__).parent / "data" / "error" / filename).with_suffix(".mxsl")
    mtlx_path = (Path(__file__).parent / "data" / "error" / filename).with_suffix(".mtlx")

    with pytest.raises(CompileError):
        mxslc.compile_file(mxsl_path, main_func=main_function, main_args=main_args)

    mtlx_path.unlink(missing_ok=True)
