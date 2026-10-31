"""Cook logic and parameter callbacks for the SLX Material LOP."""

import hashlib
import os
import re

import hou
from pxr import Sdf, UsdGeom, UsdShade

from slx_houdini import NODE_TYPE_NAME, compiler, editor, examples, file_watcher, global_parameters

MATERIALX_MATERIALS_PATH = "/MaterialX/Materials"
ERROR_LINE_PATTERN = re.compile(r"\bline (\d+)")

_compile_results = {}  # node session id -> CompileResult
_declared_globals = {}  # node session id -> (source and settings key, [GlobalParameter])
_scheduled_global_updates = set()  # session ids of nodes whose global parameters change once their cook ends
_automatic_materialx_version = None
_is_runtime_created = False


class SourceError(Exception):
    """A problem with the node's SLX source or settings, reported as an error on the node."""


class CompileResult(object):
    """What a node's last cook produced, kept for its parameter callbacks."""

    def __init__(self, xml=None, material_names=(), global_parameters=None, error=None):
        self.xml = xml  # as compiled, before function nodes are expanded for USD
        self.material_names = list(material_names)
        self.global_parameters = global_parameters
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
        xml, parameters = compile_node(node)
        usd_xml = prepare_materialx_for_usd(xml, get_source_directory(node), node.evalParm("expandfunctions"))
        materialx_path = write_materialx_cache(node, usd_xml)
        stage = script_node.editableStage()
        layer = Sdf.Layer.FindOrOpen(materialx_path)
        material_paths = reference_materials(stage, layer, get_material_path_prefix(node))
        if not material_paths:
            # Raised as an error because warnings from inside an asset don't show on the asset's node.
            raise SourceError("The SLX source doesn't define any materials. "
                              "Store a surfacematerial(...) in a material variable to create one.")
        assign_materials(node, stage, material_paths)
    except SourceError as e:
        _compile_results[node.sessionId()] = CompileResult(error=str(e))
        raise hou.NodeError(str(e))
    _compile_results[node.sessionId()] = CompileResult(xml, material_paths, parameters)
    if not global_parameters.has_matching_parameters(node, parameters):
        schedule_global_parameter_update(node)


def is_file_source(node):
    return node.parm("source").evalAsString() == "file"


def get_source_directory(node):
    """Returns the folder that relative paths in the node's SLX source are relative to."""
    if is_file_source(node):
        return os.path.dirname(os.path.abspath(node.evalParm("file").strip()))
    return hou.text.expandString("$HIP")


def compile_node(node):
    """Compiles the node's SLX source. Returns the MaterialX XML and the globals the source declares."""
    try:
        mxslc = compiler.import_mxslc()
    except ImportError as e:
        raise SourceError(str(e))

    if is_file_source(node):
        path = os.path.normpath(node.evalParm("file").strip())
        source_files = compiler.find_source_files(path)
        watch_source_files(node, source_files)
        if not os.path.isfile(path):
            raise SourceError("SLX file not found: {}. Press Create File to start from a template.".format(path))
        source, is_file, compile_function = path, True, mxslc.compile_file_to_string
        # A file's globals change when it, or a file it includes, is saved.
        source_key = tuple((p, file_watcher.get_modified_time(p)) for p in source_files)
        settings = get_compile_settings(node, mxslc, search_directories=[])
    else:
        file_watcher.unwatch_files(node)
        source = node.parm("code").unexpandedString()
        source_key, is_file, compile_function = source, False, mxslc.compile_string_to_string
        # Inline code has no file of its own, so #include and #library paths are resolved from $HIP.
        settings = get_compile_settings(node, mxslc, search_directories=[hou.text.expandString("$HIP")])

    parameters = get_declared_globals(node, mxslc, source, is_file, settings, source_key)
    options = create_compile_options(mxslc, settings, global_parameters.get_global_values(node, parameters))
    return run_compiler(compile_function, source, options), parameters


def watch_source_files(node, source_files):
    if node.evalParm("autorecompile"):
        file_watcher.watch_files(node, source_files, recompile_and_report)
    else:
        file_watcher.unwatch_files(node)


def get_declared_globals(node, mxslc, source, is_file, settings, source_key):
    """Returns the globals the source declares, running the source again only when it or the settings changed.

    That keeps changes to a global's parameter, such as dragging a slider, down to one compile.
    """
    key = (source_key, repr(sorted(settings.items())))
    cached = _declared_globals.get(node.sessionId())
    if cached is not None and cached[0] == key:
        return cached[1]
    options = create_compile_options(mxslc, settings, {})
    try:
        parameters = global_parameters.get_global_parameters(mxslc, source, options, is_file)
    except RuntimeError as e:
        raise SourceError("SLX compile error: {}".format(e))
    _declared_globals[node.sessionId()] = (key, parameters)
    return parameters


def get_compile_settings(node, mxslc, search_directories):
    """Returns the node's compile options, apart from global values, as CompileOptions keyword arguments."""
    macros = []
    for i in range(1, node.evalParm("macros") + 1):
        name = node.evalParm("macroname{}".format(i)).strip()
        value = node.evalParm("macrovalue{}".format(i))
        if name:
            macros.append((name, value) if value else name)

    entry_function = node.evalParm("entryfunction").strip() or None
    settings = {
        "macros": macros,
        "search_directories": search_directories,
        # Globals without a parameter, such as structs, use their initializers.
        "error_on_missing_globals": False,
        "error_on_unused_globals": False,
        "func_name": entry_function,
        "func_args": compiler.parse_value_list(node.evalParm("entryarguments")) if entry_function else [],
        "reduce_graph": bool(node.evalParm("reducegraph")),
    }
    version = get_materialx_version(node, mxslc)
    if version:
        settings["version"] = version
    return settings


def create_compile_options(mxslc, settings, global_values):
    if compiler.has_sequence_value(list(global_values.values()) + settings["func_args"]):
        create_runtime(mxslc)
    try:
        return mxslc.CompileOptions(globals=global_values, **settings)
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


def prepare_materialx_for_usd(xml, source_directory, expand_functions):
    """Returns the compiled MaterialX changed to work from the cache directory, in USD and Houdini's renderers.

    - Relative file paths become absolute, from source_directory. USD would resolve them from the cache directory.
    - With expand_functions, each node made from a non-inline SLX function is replaced with the nodes inside it.
      Karma and Houdini's viewport only render MaterialX's standard nodes. They ignore the custom NodeDefs that
      SLX functions compile to, even though USD records which file defines them.
    """
    import MaterialX as mx
    document = mx.createDocument()
    mx.readFromXmlString(document, xml)
    if expand_functions and document.getNodeDefs():
        document.flattenSubgraphs()
        for graph in document.getNodeGraphs():
            if not graph.getNodeDefString():
                graph.flattenSubgraphs()
    for element in document.traverseTree():
        if element.isA(mx.Input) and element.getType() == "filename" and element.hasValueString():
            element.setValueString(make_absolute_path(element.getResolvedValueString(), source_directory))
    return mx.writeToXmlString(document)


def make_absolute_path(path, directory):
    path = path.replace("\\", "/")
    if not path or os.path.isabs(path) or re.match(r"^[A-Za-z][A-Za-z0-9+.-]*:", path):
        return path  # absolute, empty, or with a drive letter or URL scheme
    return os.path.join(directory, path).replace("\\", "/")


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
    update_global_parameters(node)
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


def update_global_parameters(node):
    """Cooks node and gives it a parameter for each global its SLX source declares.

    Cooks do this themselves when Houdini's UI is running. Scripts in hython can call this instead.
    """
    result = get_last_result(node)
    if result is None or result.global_parameters is None:
        return
    if not global_parameters.has_matching_parameters(node, result.global_parameters):
        global_parameters.update_parameters(node, result.global_parameters)
        # The last cook couldn't read parameters that didn't exist, so Houdini wouldn't recook when they change.
        recompile(node)


def schedule_global_parameter_update(node):
    """Updates node's global parameters once its cook ends, because Houdini forbids changing them during it.

    This needs Houdini's UI, which runs the update from its event loop.
    """
    if not hou.isUIAvailable() or node.sessionId() in _scheduled_global_updates:
        return
    import hdefereval
    session_id = node.sessionId()
    _scheduled_global_updates.add(session_id)

    def update():
        _scheduled_global_updates.discard(session_id)
        scheduled_node = hou.nodeBySessionId(session_id)
        if scheduled_node is not None:
            update_global_parameters(scheduled_node)

    hdefereval.executeDeferred(update)


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


def on_load_example_pressed(kwargs):
    node = kwargs["node"]
    directory = examples.find_examples_directory()
    paths = examples.get_examples(directory) if directory else []
    if not paths:
        hou.ui.displayMessage("The SLX examples weren't found. They come with the plugin's release download, and "
                              "with the ShadingLanguageX repository.", title="Load Example")
        return
    names = [os.path.splitext(os.path.basename(p))[0] for p in paths]
    chosen = hou.ui.selectFromList(names, exclusive=True, title="Load Example", column_header="Example",
                                   message="The example is copied to $HIP/slx_examples, where you can edit it.")
    if not chosen:
        return
    copied_path = examples.copy_example(paths[chosen[0]], hou.text.expandString("$HIP/slx_examples"))
    node.parm("source").set("file")
    node.parm("file").set("$HIP/slx_examples/" + os.path.basename(copied_path))
    recompile_and_report(node)


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
