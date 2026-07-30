from pathlib import Path
import mxslc

from groundtruth import RANDOMFLOAT_OUTPUT

def test_compile_string_to_string():
    result = mxslc.compile_string_to_string('float f = randomfloat() + 1.0;')

    assert isinstance(result, str)
    assert result == RANDOMFLOAT_OUTPUT
    

def test_compile_file_to_string(tmp_path):
    source_path = tmp_path / "simple.mxsl"
    source_path.write_text("float f = randomfloat() + 1.0;", encoding="utf-8")

    result = mxslc.compile_file_to_string(source_path)

    assert isinstance(result, str)
    assert result == RANDOMFLOAT_OUTPUT


def test_compile_string_to_file(tmp_path):
    output_path = tmp_path / "output.mtlx"

    returned_output_path = mxslc.compile_string_to_file("float f = randomfloat() + 1.0;", output_path)

    assert isinstance(returned_output_path, Path)
    assert returned_output_path == output_path
    assert output_path.exists()
    assert output_path.read_text(encoding="utf-8") == RANDOMFLOAT_OUTPUT


def test_compile_file_to_file_1(tmp_path):
    source_path = tmp_path / "simple.mxsl"
    output_path = tmp_path / "out.mtlx"

    source_path.write_text("float f = randomfloat() + 1.0;", encoding="utf-8")

    returned_output_path = mxslc.compile_file_to_file(source_path, output_path)

    assert isinstance(returned_output_path, Path)
    assert returned_output_path == output_path
    assert output_path.exists()
    assert output_path.read_text(encoding="utf-8") == RANDOMFLOAT_OUTPUT


def test_compile_file_to_file_2(tmp_path):
    source_path = tmp_path / "simple.mxsl"
    output_path = tmp_path / "simple.mtlx"

    source_path.write_text("float f = randomfloat() + 1.0;", encoding="utf-8")

    returned_output_path = mxslc.compile_file_to_file(source_path)

    assert isinstance(returned_output_path, Path)
    assert returned_output_path == output_path
    assert output_path.exists()
    assert output_path.read_text(encoding="utf-8") == RANDOMFLOAT_OUTPUT
