from pathlib import Path
from warnings import warn

import MaterialX as mx
import pytest

import mxslc
from mxslc import Macro

_overwrite_all_expected = False


@pytest.mark.parametrize("filename, main_function, main_args, overwrite_expected", [
    ("main_function/main_function_1", None, [], False),
    ("main_function/main_function_2", None, [], False),
    ("main_function/main_function_3", "_main", [], False),
    ("main_function/main_function_4", "_main", [], False),
    ("main_function/main_function_5", None, [], False),
    ("main_function/main_function_6", None, [0.2, 0.5, 0.8], False),
    ("main_function/main_function_7", None, [1.0, Path("butterfly1.png")], False),
    ("main_function/main_function_8", "my_function", [mx.Vector2(), mx.Color3(1.0, 0.0, 0.0)], False),
    ("main_function/main_function_9", None, [], False),
    ("main_function/main_function_10", "main", [], False),
])
def test_main_function(filename: str, main_function: str | None, main_args: list, overwrite_expected: bool) -> None:
    mxsl_path     = (Path(__file__).parent / "data" / "mxsl" / filename).with_suffix(".mxsl")
    actual_path   = (Path(__file__).parent / "data" / "mxsl" / filename).with_suffix(".mtlx")
    expected_path = (Path(__file__).parent / "data" / "mtlx" / filename).with_suffix(".mtlx")

    if overwrite_expected or _overwrite_all_expected:
        warn(f"Expected data for {filename} is being overwritten.")
        actual_path = expected_path

    mxslc.compile_file(mxsl_path, actual_path, main_func=main_function, main_args=main_args)

    with open(actual_path, "r") as f:
        actual = f.read()

    with open(expected_path, "r") as f:
        expected = f.read()

    if not (overwrite_expected or _overwrite_all_expected):
        actual_path.unlink()

    assert actual.replace("\\", "/") == expected.replace("\\", "/")


@pytest.mark.parametrize("filename, add_macros, overwrite_expected", [
    ("add_macros/add_macros_1", ["A"], False),
    ("add_macros/add_macros_2", ["B"], False),
    ("add_macros/add_macros_3", [Macro("GAMMA", "2.2")], False),
    ("add_macros/add_macros_4", [Macro("GAMMA", "2.2"), Macro("IMAGE_PATH", '"textures/butterfly1.png"')], False),
])
def test_additional_macros(filename: str, add_macros: list[Macro], overwrite_expected: bool) -> None:
    mxsl_path     = (Path(__file__).parent / "data" / "mxsl" / filename).with_suffix(".mxsl")
    actual_path   = (Path(__file__).parent / "data" / "mxsl" / filename).with_suffix(".mtlx")
    expected_path = (Path(__file__).parent / "data" / "mtlx" / filename).with_suffix(".mtlx")

    if overwrite_expected or _overwrite_all_expected:
        warn(f"Expected data for {filename} is being overwritten.")
        actual_path = expected_path

    mxslc.compile_file(mxsl_path, actual_path, add_macros=add_macros)

    with open(actual_path, "r") as f:
        actual = f.read()

    with open(expected_path, "r") as f:
        expected = f.read()

    if not (overwrite_expected or _overwrite_all_expected):
        actual_path.unlink()

    assert actual.replace("\\", "/") == expected.replace("\\", "/")


@pytest.mark.parametrize("filename, add_include_dirs, overwrite_expected", [
    ("add_include_dirs/add_include_dirs_1", [Path(__file__).parent / "data" / "add_include_dir"], False),
])
def test_additional_include_dirs(filename: str, add_include_dirs: list[Path], overwrite_expected: bool) -> None:
    mxsl_path     = (Path(__file__).parent / "data" / "mxsl" / filename).with_suffix(".mxsl")
    actual_path   = (Path(__file__).parent / "data" / "mxsl" / filename).with_suffix(".mtlx")
    expected_path = (Path(__file__).parent / "data" / "mtlx" / filename).with_suffix(".mtlx")

    if overwrite_expected or _overwrite_all_expected:
        warn(f"Expected data for {filename} is being overwritten.")
        actual_path = expected_path

    mxslc.compile_file(mxsl_path, actual_path, add_include_dirs=add_include_dirs)

    with open(actual_path, "r") as f:
        actual = f.read()

    with open(expected_path, "r") as f:
        expected = f.read()

    if not (overwrite_expected or _overwrite_all_expected):
        actual_path.unlink()

    assert actual.replace("\\", "/") == expected.replace("\\", "/")
