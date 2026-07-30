def test_import_mxslc():
    import mxslc

    assert mxslc is not None


def test_expected_api_is_exposed():
    import mxslc

    # classes
    assert hasattr(mxslc, "CompileOptions")
    assert hasattr(mxslc, "Variable")
    assert hasattr(mxslc, "Macro")
    assert hasattr(mxslc, "Decompiler")

    # functions
    assert hasattr(mxslc, "create_variable")

    assert hasattr(mxslc, "compile_string_to_string")
    assert hasattr(mxslc, "compile_file_to_string")
    assert hasattr(mxslc, "compile_file_to_file")
    assert hasattr(mxslc, "compile_string_to_file")

    assert hasattr(mxslc, "decompile_string_to_string")
    assert hasattr(mxslc, "decompile_file_to_string")
    assert hasattr(mxslc, "decompile_file_to_file")
    assert hasattr(mxslc, "decompile_string_to_file")
