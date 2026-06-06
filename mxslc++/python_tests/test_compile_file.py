import mxslc


def test_compile_file_to_string_returns_text(tmp_path):
    source_path = tmp_path / "simple.mxsl"
    source_path.write_text("float f = 1.0 + 1.0;", encoding="utf-8")

    result = mxslc.compile_file_to_string(source_path)

    assert isinstance(result, str)
    assert result.strip()


def test_compile_file_to_string_accepts_options(tmp_path):
    source_path = tmp_path / "simple.mxsl"
    source_path.write_text("float f = 1.0 + 1.0;", encoding="utf-8")

    options = mxslc.CompileOptions()
    result = mxslc.compile_file_to_string(source_path, options)

    assert isinstance(result, str)
    assert result.strip()


def test_compile_string_to_file_writes_output(tmp_path):
    output_path = tmp_path / "out.mtlx"

    mxslc.compile_string_to_file("float f = 1.0 + 1.0;", output_path)

    assert output_path.exists()
    assert output_path.read_text(encoding="utf-8").strip()


def test_compile_file_to_file_writes_output(tmp_path):
    source_path = tmp_path / "simple.mxsl"
    output_path = tmp_path / "out.mtlx"

    source_path.write_text("float f = 1.0 + 1.0;", encoding="utf-8")

    mxslc.compile_file_to_file(source_path, output_path)

    assert output_path.exists()
    assert output_path.read_text(encoding="utf-8").strip()
