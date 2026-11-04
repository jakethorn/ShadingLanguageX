"""Turns MaterialX-based UsdShade materials back into MaterialX documents.

Material Library LOPs author each MaterialX VOP as a Shader prim whose info:id names its NodeDef, so a document
can be rebuilt by looking those NodeDefs up in Houdini's MaterialX libraries. This works for every MaterialX
material in a stage, including Karma Material Builders, which Houdini's own VOP exporter (vop2mtlx) rejects.
"""

import hou
from pxr import Gf, Sdf, UsdShade

# Render contexts whose material outputs hold MaterialX shaders, in order of preference.
RENDER_CONTEXTS = ("mtlx", "kma")
TERMINALS = ("surface", "displacement", "volume")

_library = None


def get_materialx_library():
    """Returns a document holding Houdini's MaterialX NodeDefs, for looking them up by name."""
    global _library
    if _library is None:
        import MaterialX as mx
        _library = mx.createDocument()
        search_path = mx.FileSearchPath(hou.text.expandString("$HFS/houdini/materialx"))
        mx.loadLibraries(["libraries"], search_path, _library)
    return _library


def find_terminals(material):
    """Returns {terminal name: connected Shader output} for the first MaterialX render context with any."""
    for context in RENDER_CONTEXTS:
        terminals = {}
        for name in TERMINALS:
            output = material.GetOutput("{}:{}".format(context, name))
            sources = output.GetConnectedSources()[0] if output else []
            if sources:
                terminals[name] = sources[0]
        if terminals:
            return terminals
    return {}


def find_materialx_materials(stage):
    materials = [UsdShade.Material(p) for p in stage.Traverse() if p.IsA(UsdShade.Material)]
    return [m for m in materials if find_terminals(m)]


def create_materialx_xml(material):
    """Returns a MaterialX document, as XML, with material and the shader network it reaches."""
    import MaterialX as mx
    terminals = find_terminals(material)
    if not terminals:
        raise hou.OperationFailed("{} has no MaterialX shaders.".format(material.GetPath()))

    document = mx.createDocument()
    material_node = document.addMaterialNode(mx.createValidName(material.GetPrim().GetName()))
    if "surface" not in terminals and "volume" in terminals:
        material_node.setCategory("volumematerial")
    nodes = {}  # prim path -> MaterialX node
    for source in terminals.values():
        shader_node = add_shader_node(document, material, source.source.GetPrim(), nodes)
        material_node.addInput(shader_node.getType(), shader_node.getType()).setConnectedNode(shader_node)
    return mx.writeToXmlString(document)


def add_shader_node(document, material, prim, nodes):
    import MaterialX as mx
    if prim.GetPath() in nodes:
        return nodes[prim.GetPath()]
    if not prim.IsA(UsdShade.Shader):
        message = "{} is a {}, which can't be converted yet.".format(prim.GetPath(), prim.GetTypeName())
        raise hou.OperationFailed(message)

    shader = UsdShade.Shader(prim)
    node_def_name = shader.GetIdAttr().Get()
    node_def = get_materialx_library().getNodeDef(node_def_name)
    if node_def is None:
        message = "{} uses {}, which isn't a standard MaterialX node.".format(prim.GetPath(), node_def_name)
        raise hou.OperationFailed(message)

    node = document.addNode(node_def.getNodeString(), mx.createValidName(prim.GetName()), node_def.getType())
    nodes[prim.GetPath()] = node

    for usd_input in shader.GetInputs():
        node_def_input = node_def.getActiveInput(usd_input.GetBaseName())
        if node_def_input is None:
            continue  # Houdini adds inputs MaterialX doesn't define, e.g. for viewport previews
        add_input(document, material, node, node_def_input, usd_input, nodes)
    return node


def add_input(document, material, node, node_def_input, usd_input, nodes):
    sources = usd_input.GetConnectedSources()[0]
    if sources and sources[0].source.GetPrim() != material.GetPrim():
        source = sources[0]
        upstream = add_shader_node(document, material, source.source.GetPrim(), nodes)
        mx_input = node.addInput(node_def_input.getName(), node_def_input.getType())
        mx_input.setConnectedNode(upstream)
        if upstream.getType() == "multioutput":
            mx_input.setOutputString(source.sourceName)
        return

    # An input connected to the material reads one of the material's interface inputs, so use that value.
    attribute = usd_input.GetValueProducingAttributes()[0] if sources else usd_input.GetAttr()
    if attribute and attribute.HasAuthoredValue():
        mx_input = node.addInput(node_def_input.getName(), node_def_input.getType())
        mx_input.setValueString(format_value(attribute.Get()))
        color_space = attribute.GetColorSpace() if attribute.HasColorSpace() else ""
        if color_space:
            mx_input.setColorSpace(color_space)


def format_value(value):
    """Formats a USD attribute value the way MaterialX writes values, e.g. "0.5, 0.25, 1"."""
    if isinstance(value, bool):
        return "true" if value else "false"
    if isinstance(value, float):
        return "{:.7g}".format(value)
    if isinstance(value, Sdf.AssetPath):
        return value.path
    if isinstance(value, (Gf.Matrix3d, Gf.Matrix3f, Gf.Matrix4d, Gf.Matrix4f)):
        return ", ".join(format_value(float(v)) for row in value for v in row)
    if hasattr(value, "__len__") and not isinstance(value, str):
        return ", ".join(format_value(float(v) if isinstance(v, float) else v) for v in value)
    return str(value)
