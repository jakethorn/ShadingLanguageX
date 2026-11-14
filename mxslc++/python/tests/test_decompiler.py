"""The Decompiler reads a str as MaterialX source if it starts with an element, otherwise as the path of a file, and
decompile_file_to_file writes the code next to the MaterialX file unless it is given a path."""

import mxslc


MTLX = """<?xml version="1.0"?>
<materialx version="1.39">
  <position name="p" type="vector3" />
</materialx>
"""

EXPECTED = "vec3 p = position();\n"


def test_decompiler_reads_source():
    assert mxslc.Decompiler(MTLX).decompile_document() == EXPECTED
    assert mxslc.Decompiler(source=MTLX).decompile_document() == EXPECTED


def test_decompiler_reads_files(tmp_path):
    path = tmp_path / "example.mtlx"
    path.write_text(MTLX)
    assert mxslc.Decompiler(str(path)).decompile_document() == EXPECTED
    assert mxslc.Decompiler(path).decompile_document() == EXPECTED
    assert mxslc.Decompiler(src_path=path).decompile_document() == EXPECTED
    assert mxslc.Decompiler(src_path=str(path)).decompile_document() == EXPECTED


def test_decompile_file_to_file(tmp_path):
    src_path = tmp_path / "example.mtlx"
    src_path.write_text(MTLX)

    # the code is written next to the MaterialX file by default
    assert mxslc.decompile_file_to_file(src_path) == tmp_path / "example.mxsl"
    assert (tmp_path / "example.mxsl").read_text() == EXPECTED

    dst_path = tmp_path / "other.mxsl"
    assert mxslc.decompile_file_to_file(src_path, dst_path) == dst_path
    assert dst_path.read_text() == EXPECTED
