def test_import_mxslc():
    import mxslc

    assert mxslc is not None


def test_expected_api_is_exposed():
    import mxslc

    assert hasattr(mxslc, "CompileOptions")
    assert hasattr(mxslc, "compile_string_to_string")
    assert hasattr(mxslc, "compile_file_to_string")
    assert hasattr(mxslc, "compile_file_to_file")
    assert hasattr(mxslc, "compile_string_to_file")
