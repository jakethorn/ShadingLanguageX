"""Tests for preserving field/port names during MXSL <-> MTLX conversion."""

import mxslc
import pytest


# ---------------------------------------------------------------------------
#  MXSL -> MTLX: field names should appear as MTLX output port names
# ---------------------------------------------------------------------------

MXSL_MULTI_OUTPUT = """\
{color3 out_color, float out_roughness} brass1 =>
{
    color3 image_color = tiledimage(file = "test.jpg", uvtiling = vec2{1, 1});
    float image_roughness = tiledimage(file = "test_r.jpg", uvtiling = vec2{1, 1});
    return {image_color, image_roughness};
}
surfaceshader SR = UsdPreviewSurface(diffuseColor = brass1.out_color, roughness = brass1.out_roughness);
material M = surfacematerial(surfaceshader = SR);
"""


def test_compile_preserves_field_names():
    """Field names like 'out_color' should appear in MTLX output port names."""
    result = mxslc.compile_string_to_string(MXSL_MULTI_OUTPUT)

    # The nodegraph outputs should use field names, not positional out__N
    assert 'output="out_color"' in result, \
        "MTLX input reference should use field name 'out_color', not positional"
    assert 'output="out_roughness"' in result, \
        "MTLX input reference should use field name 'out_roughness', not positional"


def test_compile_no_positional_names():
    """Positional out__0/out__1 should NOT appear when field names are present."""
    result = mxslc.compile_string_to_string(MXSL_MULTI_OUTPUT)

    assert 'out__0' not in result, \
        "Positional name 'out__0' should not be used when field names exist"
    assert 'out__1' not in result, \
        "Positional name 'out__1' should not be used when field names exist"


# ---------------------------------------------------------------------------
#  MTLX -> MXSL: names preserved, with MXSL keyword conflicts handled
# ---------------------------------------------------------------------------

MTLX_WITH_RESERVED_NAME = """\
<?xml version="1.0"?>
<materialx version="1.39">
  <nodegraph name="root_graph">
    <texcoord name="texcoord_vector3" type="vector2">
      <input name="index" type="integer" value="0" />
    </texcoord>
    <grid name="grid_color3" type="color3">
      <input name="texcoord" type="vector2" nodename="texcoord_vector3" />
    </grid>
    <output name="out" type="color3" nodename="grid_color3" />
    <crosshatch name="crosshatch_color3" type="color3">
      <input name="texcoord" type="vector2" nodename="texcoord_vector3" />
    </crosshatch>
    <output name="my_out2" type="color3" nodename="crosshatch_color3" />
    <max name="union_color3" type="color3">
      <input name="in1" type="color3" nodename="grid_color3" />
      <input name="in2" type="color3" nodename="crosshatch_color3" />
    </max>
    <output name="my_out3" type="color3" nodename="union_color3" />
  </nodegraph>
  <UsdPreviewSurface name="SR" type="surfaceshader">
    <input name="diffuseColor" type="color3" nodegraph="root_graph" output="out" />
  </UsdPreviewSurface>
  <surfacematerial name="M" type="material">
    <input name="surfaceshader" type="surfaceshader" nodename="SR" />
  </surfacematerial>
</materialx>
"""


def test_decompile_renames_reserved_keyword():
    """'out' is an MXSL keyword, should be renamed to 'out1' in decompiled output."""
    result = mxslc.decompile_string_to_string(MTLX_WITH_RESERVED_NAME)

    # The type declaration should use 'out1' not 'out' for the first output
    assert '{color3 out1, color3 my_out2, color3 my_out3}' in result, \
        "Reserved name 'out' should be renamed to 'out1' in type declaration"


def test_decompile_reference_matches_renamed_output():
    """References to the renamed output should use the same safe name."""
    result = mxslc.decompile_string_to_string(MTLX_WITH_RESERVED_NAME)

    # The reference should match the renamed output name
    assert 'diffuseColor = root_graph.out1' in result, \
        "Reference to renamed output 'out' should use 'root_graph.out1'"
    assert 'root_graph.out' not in result.split('diffuseColor')[0], \
        "Reference should not use raw 'out' when it's a reserved keyword"


# ---------------------------------------------------------------------------
#  Round-trip: compile then decompile preserves field names
# ---------------------------------------------------------------------------

def test_round_trip_preserves_field_names():
    """Compiling MXSL to MTLX and back should preserve field names."""
    mtlx = mxslc.compile_string_to_string(MXSL_MULTI_OUTPUT)
    mxsl_back = mxslc.decompile_string_to_string(mtlx)

    assert 'out_color' in mxsl_back, \
        "Field name 'out_color' should survive round-trip"
    assert 'out_roughness' in mxsl_back, \
        "Field name 'out_roughness' should survive round-trip"


# ---------------------------------------------------------------------------
#  Unnamed fields fall back to positional naming
# ---------------------------------------------------------------------------

MXSL_UNNAMED_FIELDS = """\
{color3, float} unnamed =>
{
    color3 c = tiledimage(file = "test.jpg");
    float f = 0.5;
    return {c, f};
}
"""


def test_unnamed_fields_use_positional_names():
    """Fields without names should fall back to positional out__N naming."""
    result = mxslc.compile_string_to_string(MXSL_UNNAMED_FIELDS)

    # Unnamed fields should use positional names
    assert 'out__0' in result, \
        "Unnamed fields should use positional 'out__0' as fallback"
    assert 'output name="out__0"' in result, \
        "Input reference should also use positional 'out__0'"
