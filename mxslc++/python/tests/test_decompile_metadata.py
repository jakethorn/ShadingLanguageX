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
    'color3 out = image(@colorspace "srgb_texture" @unit "feet" @unittype "distance" file = "textures/albedo.tif");\n'
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
    <input name="transmission_color" type="color3" value="0.969, 0.996, 0.997" unit="feet" unittype="distance" />
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

    # Input-level metadata is emitted inline before the input argument
    assert '@colorspace "lin_rec709"' in result
    assert '@unit "feet"' in result
    assert '@unittype "distance"' in result

    # Structural attributes are not emitted
    for structural in ("@name", "@type", "@value", "@nodename"):
        assert structural not in result, f"Structural attribute {structural!r} should not be emitted"

    print("Decompiled result:\n", result)

    # Check that we can compile the decompiled result back to MaterialX and that it matches the original
    compiled = mxslc.compile_string_to_string(result)
    print("Compiled result:\n", compiled)
    assert compiled == MTLX_SURFACE_SHADER, "Recompiled MaterialX does not match original"


MXSL_NODEDEF_WITH_METADATA = '''@nodegroup "math"
@version "1.0"
@isdefaultversion "true"
@doc "This is a custom definition that adds two color3 inputs together and returns the result."
{ color3 out1, color3 out2 } mydefinition(
                    @uiname "Input 1"  @uifolder "Inputs" @uimin "0.0" @uimax "1.0" @uisoftmin "0.0" @uisoftmax "1.0" @uistep "0.01"
                    color3 input1 = {0.0, 0.0, 0.0}, 
                    @uiname "Input 2"  @uifolder "Inputs" @uimin "0.0" @uimax "1.0" @uisoftmin "0.0" @uisoftmax "1.0" @uistep "0.01"
                    color3 input2)
{
    color3 graph_node_1 = (input1 + input2);
    color3 graph_node_2 = (input1 - input2);
    return { graph_node_1, graph_node_2 };
}
'''

# NodeDef-level metadata that must survive the roundtrip
NODEDEF_METADATA = [
    '@nodegroup "math"',
    '@version "1.0"',
    '@isdefaultversion "true"',
    '@doc "This is a custom definition that adds two color3 inputs together and returns the result."',
]

# Input-level metadata that must survive the roundtrip
INPUT_METADATA = [
    '@uiname "Input 1"',
    '@uifolder "Inputs"',
    '@uimin "0.0"',
    '@uimax "1.0"',
    '@uisoftmin "0.0"',
    '@uisoftmax "1.0"',
    '@uistep "0.01"',
    '@uiname "Input 2"',
]


def test_nodedef_metadata_roundtrip():
    """NodeDef and input-level metadata survives a compile -> decompile roundtrip."""
    # Compile MXSL -> MTLX
    mtlx = mxslc.compile_string_to_string(MXSL_NODEDEF_WITH_METADATA)
    print("Compiled MTLX:\n", mtlx)

    # NodeDef-level metadata is written onto the nodedef element
    assert 'nodegroup="math"' in mtlx
    assert 'version="1.0"' in mtlx
    assert 'isdefaultversion="true"' in mtlx
    assert 'doc="This is a custom definition that adds two color3 inputs together and returns the result."' in mtlx

    # Input-level metadata is written onto the nodedef input elements
    assert 'uiname="Input 1"' in mtlx
    assert 'uifolder="Inputs"' in mtlx
    assert 'uimin="0.0"' in mtlx
    assert 'uimax="1.0"' in mtlx
    assert 'uisoftmin="0.0"' in mtlx
    assert 'uisoftmax="1.0"' in mtlx
    assert 'uistep="0.01"' in mtlx
    assert 'uiname="Input 2"' in mtlx

    # Decompile MTLX -> MXSL
    mxsl = mxslc.decompile_string_to_string(mtlx)
    print("Decompiled MXSL:\n", mxsl)

    # NodeDef-level metadata is preserved as @ declarations
    for metadata in NODEDEF_METADATA:
        assert metadata in mxsl, f"Missing NodeDef metadata {metadata!r}:\n{mxsl}"

    # Input-level metadata is preserved inline on the function parameters
    for metadata in INPUT_METADATA:
        assert metadata in mxsl, f"Missing input metadata {metadata!r}:\n{mxsl}"

    # Structural attributes are not emitted as @ declarations
    for structural in ("@name", "@node ", "@nodedef", "@type", "@value"):
        assert structural not in mxsl, f"Structural attribute {structural!r} should not be emitted:\n{mxsl}"

    # Compile MXSL -> MTLX again and check that it matches the original MTLX
    mtlx_roundtrip = mxslc.compile_string_to_string(mxsl)
    print("Recompiled MTLX:\n", mtlx_roundtrip)
    assert mtlx_roundtrip == mtlx, "Recompiled MTLX does not match original MTLX"

    # Test MaterialX validation
    try:
        import MaterialX as mx

        doc = mx.createDocument()
        stdlib = mx.createDocument()
        mx.loadLibraries(mx.getDefaultDataLibraryFolders(), mx.getDefaultDataSearchPath(), stdlib)
        doc.setDataLibrary(stdlib)    
        mx.readFromXmlString(doc, mtlx_roundtrip)
        result, error = doc.validate()
        print("MaterialX validation result:", result, error)
        assert result, f"MaterialX validation failed: {error}"

        # Check for nodedef + metadata in the document, accumulating all issues
        # so that every failure is reported at once after the loop.
        issues: list[str] = []

        def check(condition: bool, message: str) -> None:
            if not condition:
                issues.append(message)

        doc_node_defs = [nd for nd in doc.getNodeDefs() if len(nd.getSourceUri()) == 0]
        check(len(doc_node_defs) == 1, f"Expected 1 custom NodeDef in the document, found {len(doc_node_defs)}")

        for nodedef in doc_node_defs:
            name = nodedef.getName()

            # NodeDef identity and metadata
            check(nodedef.getNodeString() == 'mydefinition',
                  f"NodeDef '{name}' node is {nodedef.getNodeString()!r}, expected 'mydefinition'")
            check(nodedef.getAttribute('nodegroup') == 'math',
                  f"NodeDef '{name}' nodegroup is {nodedef.getAttribute('nodegroup')!r}, expected 'math'")
            check(nodedef.getVersionString() == '1.0',
                  f"NodeDef '{name}' version is {nodedef.getVersionString()!r}, expected '1.0'")
            check(nodedef.getAttribute('isdefaultversion') == 'true',
                  f"NodeDef '{name}' isdefaultversion is {nodedef.getAttribute('isdefaultversion')!r}, expected 'true'")
            check(nodedef.getDocString() == 'This is a custom definition that adds two color3 inputs together and returns the result.',
                  f"NodeDef '{name}' doc is {nodedef.getDocString()!r}")

            # Implementation must exist and be the expected NodeGraph
            implementation = nodedef.getImplementation()
            check(implementation is not None, f"NodeDef '{name}' has no implementation")
            if implementation is not None:
                check(implementation.getName() == 'NG_mydefinition',
                      f"NodeDef '{name}' implementation is {implementation.getName()!r}, expected 'NG_mydefinition'")
                check(implementation.isA(mx.NodeGraph),
                      f"NodeDef '{name}' implementation is not a NodeGraph")
                if implementation.isA(mx.NodeGraph):
                    impl_nodes = {n.getName(): n for n in implementation.getNodes()}
                    check(set(impl_nodes) == {'graph_node_1', 'graph_node_2'},
                          f"NodeGraph 'NG_mydefinition' nodes are {sorted(impl_nodes)}, expected ['graph_node_1', 'graph_node_2']")
                    for node_name, expected_category in (('graph_node_1', 'add'), ('graph_node_2', 'subtract')):
                        node = impl_nodes.get(node_name)
                        check(node is not None, f"NodeGraph 'NG_mydefinition' missing node '{node_name}'")
                        if node is not None:
                            check(node.getCategory() == expected_category,
                                  f"Node '{node_name}' category is {node.getCategory()!r}, expected '{expected_category}'")
                            check(node.getType() == 'color3',
                                  f"Node '{node_name}' type is {node.getType()!r}, expected 'color3'")
                            interfaces = [(i.getName(), i.getInterfaceName()) for i in node.getActiveInputs()]
                            check(interfaces == [('in1', 'input1'), ('in2', 'input2')],
                                  f"Node '{node_name}' interface inputs are {interfaces}, expected [('in1', 'input1'), ('in2', 'input2')]")

                    impl_outputs = {o.getName(): o for o in implementation.getActiveOutputs()}
                    check(set(impl_outputs) == {'out__out1', 'out__out2'},
                          f"NodeGraph 'NG_mydefinition' outputs are {sorted(impl_outputs)}, expected ['out__out1', 'out__out2']")
                    for output_name in ('out__out1', 'out__out2'):
                        out = impl_outputs.get(output_name)
                        check(out is not None, f"NodeGraph 'NG_mydefinition' missing output '{output_name}'")
                        if out is not None:
                            check(out.getType() == 'color3',
                                  f"NodeGraph output '{output_name}' type is {out.getType()!r}, expected 'color3'")

            # NodeDef inputs: names, types and UI metadata
            node_def_inputs = {i.getName(): i for i in nodedef.getActiveInputs()}
            check(set(node_def_inputs) == {'input1', 'input2'},
                  f"NodeDef '{name}' inputs are {sorted(node_def_inputs)}, expected ['input1', 'input2']")
            for input_name, expected_uiname in (('input1', 'Input 1'), ('input2', 'Input 2')):
                port = node_def_inputs.get(input_name)
                check(port is not None, f"NodeDef '{name}' missing input '{input_name}'")
                if port is not None:
                    check(port.getType() == 'color3',
                          f"NodeDef input '{input_name}' type is {port.getType()!r}, expected 'color3'")
                    for attr_name, expected_value in (('uiname', expected_uiname), ('uifolder', 'Inputs'),
                                                      ('uimin', '0.0'), ('uimax', '1.0'),
                                                      ('uisoftmin', '0.0'), ('uisoftmax', '1.0'),
                                                      ('uistep', '0.01')):
                        check(port.getAttribute(attr_name) == expected_value,
                              f"NodeDef input '{input_name}' {attr_name} is {port.getAttribute(attr_name)!r}, expected {expected_value!r}")

            # NodeDef outputs: names and types
            node_def_outputs = {o.getName(): o for o in nodedef.getActiveOutputs()}
            check(set(node_def_outputs) == {'out__out1', 'out__out2'},
                  f"NodeDef '{name}' outputs are {sorted(node_def_outputs)}, expected ['out__out1', 'out__out2']")
            for output_name in ('out__out1', 'out__out2'):
                out = node_def_outputs.get(output_name)
                check(out is not None, f"NodeDef '{name}' missing output '{output_name}'")
                if out is not None:
                    check(out.getType() == 'color3',
                          f"NodeDef output '{output_name}' type is {out.getType()!r}, expected 'color3'")

        # Report all accumulated issues at once after the loop
        assert not issues, (
            "NodeDef metadata/implementation checks failed:\n"
            + "\n".join(f"  - {issue}" for issue in issues)
        )
        print(f"NodeDef checks passed for {len(doc_node_defs)} document nodedef(s)")

        # Create downstream usage of an instance
        instance = doc.addNode('mydefinition', 'mydefinition', 'multioutput')
        nodedef = instance.getNodeDef()
        nodedef_outputs = {o.getName(): o for o in nodedef.getActiveOutputs()}
        for output_name, output in nodedef_outputs.items():
            instance.addOutput(output.getName(), output.getType())
        instance.setInputValue('input1', mx.Color3(0.5, 0.5, 0.5))
        instance.setInputValue('input2', mx.Color3(0.1, 0.2, 0.3))

        for output_name, output in nodedef_outputs.items():
          downstream = doc.addNode('standard_surface', '', 'surfaceshader')
          downstream.setConnectedNode('base_color', instance)
          input = downstream.getInput('base_color')
          input.setOutputString(output_name)

        print("Document with instance usage:\n", mx.writeToXmlString(doc))

        valid, error = doc.validate()
        if not valid:
            print("Document validation failed with the following errors:")
            print(error)
        assert valid, f"Instance usage validation failed: {error}"

    except ImportError:
        pass 
    