import mxslc


MTLX_IMAGE_WITH_METADATA = """<?xml version="1.0"?>
<materialx version="1.39">
  <image name="out" type="color3" doc="an image of a sphere">
    <input name="file" type="filename" value="textures/albedo.tif" colorspace="srgb_texture" unit="feet" unittype="distance" />
  </image>
</materialx>
"""

IMAGE_WITH_METADATA_EXPECTED = (
    '@doc "an image of a sphere"\n'
    '@file.colorspace "srgb_texture"\n'
    '@file.unit "feet"\n'
    '@file.unittype "distance"\n'
    'color3 out = image(file = "textures/albedo.tif");\n'
)

def test_emit_node_input_metadata():
    result = mxslc.decompile_string_to_string(MTLX_IMAGE_WITH_METADATA)
    assert result == IMAGE_WITH_METADATA_EXPECTED, f"Expected:\n{IMAGE_WITH_METADATA_EXPECTED!r}\nGot:\n{result!r}"


MTLX_STRUCTURAL_ATTRIBUTES = """<?xml version="1.0"?>
<materialx version="1.39">
  <image name="out" type="color3" xpos="1.5" ypos="2.5" width="240" height="120">
    <input name="file" type="filename" value="textures/albedo.tif" />
  </image>
</materialx>
"""

def test_structural_attributes():
    result = mxslc.decompile_string_to_string(MTLX_STRUCTURAL_ATTRIBUTES)
    assert result == 'color3 out = image(file = "textures/albedo.tif");\n', f"Got:\n{result!r}"

    for structural in ("@name", "@type", "@value", "@nodename", "@xpos", "@ypos", "@width", "@height"):
        assert structural not in result, f"Structural attribute {structural!r} should not be emitted"


MTLX_SURFACE_SHADER = """<?xml version="1.0"?>
<materialx version="1.39" doc="Water material document">
  <standard_surface name="Water_SS_SHD_PBM" type="surfaceshader" doc="Water documentation" uiname="Water" uifolder="Liquid">
    <input name="base_color" type="color3" value="0.969, 0.996, 0.997" colorspace="lin_rec709" />
    <input name="metalness" type="float" value="0" />
    <input name="transmission_color" type="color3" value="0.969,0.996,0.997" unit="feet" unittype="distance" />
  </standard_surface>
  <surfacematerial name="Water_SS_MAT_PBM" type="material">
    <input name="surfaceshader" type="surfaceshader" nodename="Water_SS_SHD_PBM" />
  </surfacematerial>
</materialx>
"""


def test_surface_shader_metadata_is_emitted():
    """Document, node and input level metadata are emitted as @ declarations."""
    result = mxslc.decompile_string_to_string(MTLX_SURFACE_SHADER)

    # Document-level metadata
    assert '@@doc "Water material document"' in result

    # Node-level metadata
    assert '@doc "Water documentation"' in result
    assert '@uiname "Water"' in result
    assert '@uifolder "Liquid"' in result

    # Input-level metadata
    assert '@base_color.colorspace "lin_rec709"' in result
    assert '@transmission_color.unit "feet"' in result
    assert '@transmission_color.unittype "distance"' in result

    # Structural attributes are not emitted
    for structural in ("@name", "@type", "@value", "@nodename"):
        assert structural not in result, f"Structural attribute {structural!r} should not be emitted"
