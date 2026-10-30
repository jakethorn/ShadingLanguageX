"""Cook logic and parameter callbacks for the SLX Material LOP."""

import hashlib
import os
import re

import hou
from pxr import Sdf, UsdGeom, UsdShade

from slx_houdini import NODE_TYPE_NAME, compiler, editor, file_watcher

MATERIALX_MATERIALS_PATH = "/MaterialX/Materials"
ERROR_LINE_PATTERN = re.compile(r"\bline (\d+)")

_compile_results = {}  # node session id -> CompileResult
_automatic_materialx_version = None
_is_runtime_created = False


class SourceError(Exception):
    """A problem with the node's SLX source or settings, reported as an error on the node."""


class CompileResult(object):
    """What a node's last cook produced, kept for its parameter callbacks."""

    def __init__(self, xml=None, material_names=(), error=None):
        self.xml = xml  # as compiled, before function nodes are expanded for USD
        self.material_names = list(material_names)
        self.error = error

    def error_line(self):
        match = ERROR_LINE_PATTERN.search(self.error or "")
        return int(match.group(1)) if match else None


def cook(script_node):
    """Cooks the SLX Material node containing script_node, the Python Script LOP inside the asset."""
    node = script_node.parent()
    # Reading the parameter makes Houdini recook this node when recompile() changes it.
    node.evalParm("recompile")
    _compile_results.pop(node.sessionId(), None)
    if is_file_source(node) and not node.evalParm("file").strip():
        # Like the Reference LOP, a node without a file passes its input through.
        file_watcher.unwatch_files(node)
        return
    try:
        xml = compile_node(node)
        usd_xml = expand_function_nodes(xml) if node.evalParm("expandfunctions") else xml
        materialx_path = write_materialx_cache(node, usd_xml)
        stage = script_node.editableStage()
        material_paths = reference_materials(stage, Sdf.Layer.FindOrOpen(materialx_path), get_material_path_prefix(node))
        if not material_paths:
            # Raised as an error because warnings from inside an asset don't show on the asset's node.
            raise SourceError("The SLX source doesn't define any materials. "
                              "Store a surfacematerial(...) in a material variable to create one.")
        assign_materials(node, stage, material_paths)
    except SourceError as e:
        _compile_results[node.sessionId()] = CompileResult(error=str(e))
        raise hou.NodeError(str(e))
    _compile_results[node.sessionId()] = CompileResult(xml, material_paths)


def is_file_source(node):
    return node.parm("source").evalAsString() == "file"


def compile_node(node):
    try:
        mxslc = compiler.import_mxslc()
    except ImportError as e:
        raise SourceError(str(e))

    if is_file_source(node):
        path = os.path.normpath(node.evalParm("file").strip())
        watch_source_files(node, path)
        if not os.path.isfile(path):
            raise SourceError("SLX file not found: {}. Press Create File to start from a template.".format(path))
        options = create_compile_options(node, mxslc, search_directories=[])
        return run_compiler(mxslc.compile_file_to_string, path, options)

    file_watcher.unwatch_files(node)
    # Inline code has no file of its own, so #include and #library paths are resolved from $HIP.
    options = create_compile_options(node, mxslc, search_directories=[hou.text.expandString("$HIP")])
    return run_compiler(mxslc.compile_string_to_string, node.parm("code").unexpandedString(), options)


def watch_source_files(node, path):
    if node.evalParm("autorecompile"):
        file_watcher.watch_files(node, compiler.find_source_files(path), recompile_and_report)
    else:
        file_watcher.unwatch_files(node)


def create_compile_options(node, mxslc, search_directories):
    global_values = {}
    for i in range(1, node.evalParm("globals") + 1):
        name = node.evalParm("globalname{}".format(i)).strip()
        if name:
            global_values[name] = compiler.parse_value(node.evalParm("globalvalue{}".format(i)))

    macros = []
    for i in range(1, node.evalParm("macros") + 1):
        name = node.evalParm("macroname{}".format(i)).strip()
        value = node.evalParm("macrovalue{}".format(i))
        if name:
            macros.append((name, value) if value else name)

    entry_function = node.evalParm("entryfunction").strip() or None
    entry_arguments = compiler.parse_value_list(node.evalParm("entryarguments")) if entry_function else []
    if compiler.has_sequence_value(list(global_values.values()) + entry_arguments):
        create_runtime(mxslc)

    options = {
        "macros": macros,
        "search_directories": search_directories,
        "globals": global_values,
        "error_on_missing_globals": not node.evalParm("allowmissingglobals"),
        "func_name": entry_function,
        "func_args": entry_arguments,
        "reduce_graph": bool(node.evalParm("reducegraph")),
    }
    version = get_materialx_version(node, mxslc)
    if version:
        options["version"] = version
    try:
        return mxslc.CompileOptions(**options)
    except (TypeError, ValueError, RuntimeError) as e:
        raise SourceError("Invalid compile options: {}".format(e))


def create_runtime(mxslc):
    """Works around mxslc raising "Runtime not created" for list values before anything has compiled."""
    global _is_runtime_created
    if not _is_runtime_created:
        mxslc.compile_string_to_string("")
        _is_runtime_created = True


def get_materialx_version(node, mxslc):
    global _automatic_materialx_version
    choice = node.parm("materialxversion").evalAsString()
    if choice != "auto":
        return choice
    if _automatic_materialx_version is None:
        _automatic_materialx_version = compiler.choose_materialx_version(
            compiler.get_houdini_materialx_version(), compiler.get_available_materialx_versions(mxslc)
        )
    return _automatic_materialx_version


def run_compiler(compile_function, source, options):
    try:
        return compile_function(source, options)
    except RuntimeError as e:
        raise SourceError("SLX compile error: {}".format(e))


def expand_function_nodes(xml):
    """Replaces each node made from a non-inline SLX function with the nodes inside it.

    Karma and Houdini's viewport only render MaterialX's standard nodes. They ignore the custom NodeDefs that
    SLX functions compile to, even though USD records which file defines them.
    """
    import MaterialX as mx
    document = mx.createDocument()
    mx.readFromXmlString(document, xml)
    if not document.getNodeDefs():
        return xml
    document.flattenSubgraphs()
    for graph in document.getNodeGraphs():
        if not graph.getNodeDefString():
            graph.flattenSubgraphs()
    return mx.writeToXmlString(document)


def write_materialx_cache(node, xml):
    """Writes xml to the cache directory under a name that changes with its contents and returns the path.

    A new name for new contents means USD never serves a stale copy of a layer it already loaded.
    """
    directory = node.evalParm("cachedir").strip()
    if not directory:
        raise SourceError("Set a Cache Directory for the compiled MaterialX files.")
    digest = hashlib.sha1(xml.encode("utf-8")).hexdigest()[:12]
    file_name = "{}_{}.mtlx".format(node.path().strip("/").replace("/", "_"), digest)
    path = os.path.join(directory, file_name).replace("\\", "/")
    if os.path.isfile(path):
        return path
    try:
        os.makedirs(directory, exist_ok=True)
        temp_path = "{}.{}.tmp".format(path, os.getpid())
        with open(temp_path, "w", encoding="utf-8") as f:
            f.write(xml)
        os.replace(temp_path, path)
    except OSError as e:
        raise SourceError("Could not write the compiled MaterialX to {}: {}".format(directory, e))
    return path


def get_material_path_prefix(node):
    text = node.evalParm("matpathprefix").strip().rstrip("/")
    if not text:
        return Sdf.Path.absoluteRootPath
    if not text.startswith("/") or not Sdf.Path.IsValidPathString(text):
        raise SourceError("Material Path Prefix must be an absolute primitive path, such as /materials/.")
    return Sdf.Path(text)


def reference_materials(stage, layer, prefix):
    """References every material in the compiled MaterialX layer to a prim under prefix.

    Returns {material name: prim path}. Each material in a MaterialX layer reaches its shaders and node graphs
    through internal references, so referencing the material prim alone brings in its whole network.
    """
    material_paths = {}
    materials = layer.GetPrimAtPath(MATERIALX_MATERIALS_PATH)
    if not materials:
        return material_paths
    define_scopes(stage, prefix)
    for spec in materials.nameChildren:
        path = prefix.AppendChild(spec.name)
        stage.DefinePrim(path).GetReferences().AddReference(layer.identifier, spec.path)
        material_paths[spec.name] = path
    return material_paths


def define_scopes(stage, path):
    for prefix in path.GetPrefixes():
        if not stage.GetPrimAtPath(prefix):
            UsdGeom.Scope.Define(stage, prefix)


def assign_materials(node, stage, material_paths):
    for i in range(1, node.evalParm("assignments") + 1):
        name = node.evalParm("material{}".format(i)).strip()
        pattern = node.evalParm("geometry{}".format(i)).strip()
        if not name or not pattern:
            continue
        if name not in material_paths:
            raise SourceError("Assignment {}: the SLX source has no material named '{}'.".format(i, name))
        material = UsdShade.Material(stage.GetPrimAtPath(material_paths[name]))
        rule = hou.LopSelectionRule()
        rule.setPathPattern(pattern)
        for path in rule.expandedPaths(stage=stage):
            bind_material(stage.GetPrimAtPath(path), material)


def bind_material(prim, material):
    binding_api = UsdShade.MaterialBindingAPI
    # MaterialBindingAPI became an applied schema in newer USD versions, which then expect Apply().
    binding = binding_api.Apply(prim) if hasattr(binding_api, "Apply") else binding_api(prim)
    binding.Bind(material)


def get_slx_nodes():
    node_type = hou.nodeType(hou.lopNodeTypeCategory(), NODE_TYPE_NAME)
    return node_type.instances() if node_type else ()


def recompile(node):
    """Makes node read its SLX source again the next time it cooks."""
    parm = node.parm("recompile")
    try:
        with hou.undos.disabler():
            parm.set(parm.eval() + 1)
    except hou.PermissionError:
        # Nodes inside a locked asset can't have parameters set; a forced cook still rereads the source.
        node.cook(force=True)


def recompile_and_report(node):
    """Recompiles node now, rather than when something next needs it, and shows the result in the status bar."""
    recompile(node)
    node.stage()
    result = _compile_results.get(node.sessionId())
    if not hou.isUIAvailable() or result is None:
        return
    if result.error:
        hou.ui.setStatusMessage("{}: {}".format(node.path(), result.error), severity=hou.severityType.Error)
    else:
        message = "{}: compiled {} material(s)".format(node.path(), len(result.material_names))
        hou.ui.setStatusMessage(message, severity=hou.severityType.Message)


def recompile_all():
    for node in get_slx_nodes():
        recompile_and_report(node)


def get_last_result(node):
    node.stage()
    return _compile_results.get(node.sessionId())


def on_compile_pressed(kwargs):
    recompile_and_report(kwargs["node"])


def on_open_editor_pressed(kwargs):
    node = kwargs["node"]
    path = node.evalParm("file").strip()
    if not path:
        hou.ui.displayMessage("Choose an SLX file first, or press Create File.", title="SLX Material")
        return
    if not os.path.isfile(path):
        hou.ui.displayMessage("{} doesn't exist yet. Press Create File to start from a template.".format(path),
                              title="SLX Material")
        return
    result = _compile_results.get(node.sessionId())
    editor.open_in_editor(path, result.error_line() if result else None)


def on_create_file_pressed(kwargs):
    node = kwargs["node"]
    path = node.evalParm("file").strip()
    if not path:
        chosen = hou.ui.selectFile(start_directory="$HIP/", title="Create SLX File", pattern="*.mxsl",
                                   chooser_mode=hou.fileChooserMode.Write)
        if not chosen:
            return
        if not chosen.endswith(".mxsl"):
            chosen += ".mxsl"
        node.parm("file").set(chosen)
        path = node.evalParm("file")
    if os.path.exists(path):
        hou.ui.displayMessage("{} already exists.".format(path), title="SLX Material")
        return
    directory = os.path.dirname(path)
    if directory:
        os.makedirs(directory, exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write(compiler.read_template())
    editor.open_in_editor(path)
    recompile_and_report(node)


def on_view_materialx_pressed(kwargs):
    node = kwargs["node"]
    result = get_last_result(node)
    if result is None or result.xml is None:
        hou.ui.displayMessage("There's no compiled MaterialX to show.", title="SLX Material",
                              details=result.error if result else None, severity=hou.severityType.Warning)
        return
    hou.ui.displayMessage("MaterialX compiled from {}:".format(node.path()), title="SLX Material",
                          details=result.xml, details_label="MaterialX", details_expanded=True)


def on_export_materialx_pressed(kwargs):
    node = kwargs["node"]
    result = get_last_result(node)
    if result is None or result.xml is None:
        hou.ui.displayMessage("There's no compiled MaterialX to export.", title="SLX Material",
                              details=result.error if result else None, severity=hou.severityType.Warning)
        return
    chosen = hou.ui.selectFile(start_directory="$HIP/", title="Export MaterialX", pattern="*.mtlx",
                               chooser_mode=hou.fileChooserMode.Write)
    if chosen:
        with open(hou.text.expandString(chosen), "w", encoding="utf-8") as f:
            f.write(result.xml)


def get_material_menu_items(node):
    """Returns the compiled material names for a menu, as Houdini's flat [token, label, ...] list."""
    result = _compile_results.get(node.sessionId())
    names = result.material_names if result else []
    return [item for name in names for item in (name, name)]
