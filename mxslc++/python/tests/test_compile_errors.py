import mxslc
import pytest


def test_compile_error_is_a_runtime_error():
    assert issubclass(mxslc.CompileError, RuntimeError)


def test_compile_error_lists_every_error():
    with pytest.raises(mxslc.CompileError) as error:
        mxslc.compile_string_to_string('float a = "text";\nstring b = 1.0;')

    diagnostics = error.value.diagnostics
    assert [d.line for d in diagnostics] == [1, 2]
    assert diagnostics[0].message == "Cannot assign an expression of type string to a variable or parameter of type float"
    assert str(diagnostics[1]) == "line 2: Cannot assign an expression of type float to a variable or parameter of type string"
    assert str(error.value) == "\n".join(str(d) for d in diagnostics)


def test_compile_error_file_is_the_source_path(tmp_path):
    src_path = tmp_path / "bad.mxsl"
    src_path.write_text("float a = 1.0 +;")

    with pytest.raises(mxslc.CompileError) as error:
        mxslc.compile_file_to_string(src_path)

    assert error.value.diagnostics[0].file.name == "bad.mxsl"


def test_max_errors():
    source = "\n".join(f'float v{i} = "text";' for i in range(5))
    options = mxslc.CompileOptions(max_errors=2)

    with pytest.raises(mxslc.CompileError) as error:
        mxslc.compile_string_to_string(source, options)

    assert [d.message for d in error.value.diagnostics][-1] == "Too many errors, stopping compilation"
    assert len(error.value.diagnostics) == 3
