"""Converts MaterialX materials in Houdini into SLX Material nodes."""

import os

import hou

from slx_houdini import NODE_TYPE_NAME, compiler, node_type, usd_materials


def can_convert(node):
    """Returns whether node is in a Material Library, is a Material Library, or is a LOP that loads a .mtlx file."""
    try:
        category = node.type().category()
        if category == hou.vopNodeTypeCategory():
            return find_material_vop(node) is not None
        if category == hou.lopNodeTypeCategory():
            return node.type().name() == "materiallibrary" or find_materialx_file(node) is not None
    except hou.Error:
        pass
    return False


def find_material_vop(vop):
    """Returns the VOP inside a LOP that vop belongs to, e.g. the Karma Material Builder holding an image VOP."""
    node = vop
    while node.parent() is not None and node.parent().type().category() != hou.lopNodeTypeCategory():
        node = node.parent()
    return node if node.parent() is not None else None


def find_materialx_file(lop):
    """Returns the first .mtlx file a LOP's parameters point to, e.g. a Reference LOP's file path."""
    for parm in lop.parms():
        if parm.parmTemplate().type() == hou.parmTemplateType.String and parm.eval().lower().endswith(".mtlx"):
            return parm.eval()
    return None


def convert_to_slx(node):
    """Creates SLX Material nodes with the SLX code for the materials node holds or loads, and returns them."""
    if node.type().category() == hou.vopNodeTypeCategory():
        material_vop = find_material_vop(node)
        return convert_library_materials(material_vop.parent(), [material_vop.name()])
    path = find_materialx_file(node)
    if path is not None:
        name = os.path.splitext(os.path.basename(path))[0]
        return [create_slx_node(node.parent(), decompile_file(path), name, anchor=node)]
    return convert_library_materials(node, [child.name() for child in node.children()])


def convert_library_materials(library, names):
    """Converts the materials a Material Library LOP authors for the VOPs named in names."""
    stage = library.stage()
    if stage is None:
        raise hou.OperationFailed("{} has errors, so its materials can't be read.".format(library.path()))
    materials = [m for m in usd_materials.find_materialx_materials(stage) if m.GetPrim().GetName() in names]
    if not materials:
        raise hou.OperationFailed("{} doesn't create any MaterialX materials.".format(library.path()))
    nodes = []
    for material in materials:
        code = decompile_xml(usd_materials.create_materialx_xml(material))
        nodes.append(create_slx_node(library.parent(), code, material.GetPrim().GetName(), anchor=library,
                                     offset=len(nodes)))
    return nodes


def decompile_file(path):
    try:
        return compiler.import_mxslc().decompile_file_to_string(path)
    except RuntimeError as e:
        raise hou.OperationFailed("Could not decompile {}: {}".format(path, e))


def decompile_xml(xml):
    try:
        return compiler.import_mxslc().decompile_string_to_string(xml)
    except RuntimeError as e:
        raise hou.OperationFailed("Could not decompile the material: {}".format(e))


def create_slx_node(network, code, name, anchor=None, offset=0):
    node_type.ensure_node_type()
    node = network.createNode(NODE_TYPE_NAME, hou.text.variableName("slx_{}".format(name)))
    node.parm("source").set("inline")
    node.parm("code").set(code)
    if anchor is not None:
        node.setPosition(anchor.position() + hou.Vector2(3, -offset))
    else:
        node.moveToGoodPosition()
    node.setSelected(True, clear_all_selected=offset == 0)
    return node


def convert_selected_to_slx():
    nodes = [n for n in hou.selectedNodes() if can_convert(n)]
    if not nodes:
        hou.ui.displayMessage("Select a Material Library, a material inside one, such as a Karma Material Builder, "
                              "or a LOP that loads a .mtlx file, such as a Reference LOP.", title="Convert to SLX")
        return
    # Several selected VOPs can belong to the same material, which only needs converting once.
    targets = {}
    for node in nodes:
        target = find_material_vop(node) if node.type().category() == hou.vopNodeTypeCategory() else node
        targets[target.path()] = target
    for target in targets.values():
        convert_node_with_message(target)


def convert_node_with_message(node):
    try:
        convert_to_slx(node)
    except hou.Error as e:
        hou.ui.displayMessage("Could not convert {} to SLX.".format(node.path()), details=e.instanceMessage(),
                              title="Convert to SLX", severity=hou.severityType.Error)


def import_materialx_file():
    chosen = hou.ui.selectFile(start_directory="$HIP/", title="Import MaterialX as SLX", pattern="*.mtlx",
                               chooser_mode=hou.fileChooserMode.Read)
    if not chosen:
        return
    path = hou.text.expandString(chosen)
    try:
        code = decompile_file(path)
    except hou.Error as e:
        hou.ui.displayMessage("Could not import {}.".format(path), details=e.instanceMessage(),
                              title="Import MaterialX as SLX", severity=hou.severityType.Error)
        return
    create_slx_node(get_current_lop_network(), code, os.path.splitext(os.path.basename(path))[0])


def get_current_lop_network():
    editor = hou.ui.paneTabOfType(hou.paneTabType.NetworkEditor)
    if editor is not None and editor.pwd().childTypeCategory() == hou.lopNodeTypeCategory():
        return editor.pwd()
    return hou.node("/stage")
