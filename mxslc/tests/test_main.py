from pathlib import Path

import main


def test_main_does_not_raise_compile_error():
    main._main(["invalid/path/"])


def test_main_with_args():
    mxsl_path = str((Path(__file__).parent / "data" / "mxsl" / "main_1.mxsl").resolve())
    output_path = str((Path(__file__).parent / "data" / "mtlx" / "main_1.mtlx").resolve())
    main_func = "my_function"
    main_args = ["0.6", "tangent", "some/fake/path/butterfly1.png"]
    macro1 = ["SRGB"]
    macro2 = ["GAMMA", "2.2"]
    main._main([mxsl_path, "-o", output_path, "-m", main_func, "-a", *main_args, "-d", *macro1, "-d", *macro2, "-v"])

    actual_path = Path(__file__).parent / "data" / "mtlx" / "main_1.mtlx"
    with open(actual_path, "r") as f:
        actual = f.read()

    expected_path = Path(__file__).parent / "data" / "mtlx" / "main_1_expected.mtlx"
    with open(expected_path, "r") as f:
        expected = f.read()

    assert actual.replace("\\", "/") == expected.replace("\\", "/")
