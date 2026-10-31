"""Turns the functions in a folder of SLX files into MaterialX nodes for Karma and MaterialX Material Builders.

Each non-inline SLX function compiles to a MaterialX NodeDef and NodeGraph. Houdini needs three files made from them,
in a folder that is on HOUDINI_PATH when Houdini starts:
- materialx/libraries/slx/*.mtlx, the definitions themselves,
- karmaNodeGraphs.json, which Karma needs to render the nodes (made with Houdini's mtlx2karma script), and
- otls/slx_nodes.hda, with a VOP node per function for the Tab menu (made with Houdini's mtlx2hda script).

The folder is $SLX_NODE_LIBRARY, which the plugin's package file sets and adds to HOUDINI_PATH. Its .mxsl files are
the library's source.
"""

import glob
import os
import shutil
import tempfile

import hou

from slx_houdini import compiler, editor, usd_materials

DEFAULT_LIBRARY_DIRECTORY = "$HOUDINI_USER_PREF_DIR/slx_nodes"
DEFINITIONS_DIRECTORY = os.path.join("materialx", "libraries", "slx")
KARMA_FILE_NAME = "karmaNodeGraphs.json"
HDA_PATH = os.path.join("otls", "slx_nodes.hda")
NODE_TYPE_PREFIX = "slx_"
# mtlx2hda files nodes under MaterialX/<nodegroup, title cased> in the Tab menu.
DEFAULT_NODE_GROUP = "shading language x"
TEMPLATE_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), "templates", "node_library.mxsl")
MATERIALX_TYPE_NAMES = {"float", "integer", "boolean", "string", "filename", "vector2", "vector3", "vector4",
                        "color3", "color4", "matrix33", "matrix44"}
SESSION_ONLY_LIBRARIES = "Scanned Asset Library Directories"


class LibraryBuild(object):
    """What building a node library produced."""

    def __init__(self, directory):
        self.directory = directory
        self.node_names = []
        self.errors = []  # (source file name, message)


def get_library_directory():
    return hou.text.expandString(hou.getenv("SLX_NODE_LIBRARY") or DEFAULT_LIBRARY_DIRECTORY)


def find_library_sources(directory):
    return sorted(glob.glob(os.path.join(directory, "*.mxsl")))


def build_node_library(directory=None):
    """Compiles the library's SLX files and regenerates its node definitions, Karma file and VOP nodes.

    A file that doesn't compile, or that defines a node Houdini already has, is reported and left out.
    """
    import MaterialX as mx
    directory = directory or get_library_directory()
    build = LibraryBuild(directory)
    mxslc = compiler.import_mxslc()
    houdini_library = usd_materials.get_materialx_library()
    definitions = mx.createDocument()
    definition_paths = []

    for source_path in find_library_sources(directory):
        file_name = os.path.basename(source_path)
        try:
            document = compile_definitions(mxslc, source_path)
        except RuntimeError as e:
            build.errors.append((file_name, str(e)))
            continue
        clashes = [nd.getName() for nd in document.getNodeDefs()
                   if houdini_library.getNodeDef(nd.getName()) or definitions.getNodeDef(nd.getName())]
        if clashes:
            build.errors.append((file_name, "{} already exist(s). Rename the function(s).".format(", ".join(clashes))))
            continue
        if not document.getNodeDefs():
            build.errors.append((file_name, "No non-inline functions, so it makes no nodes."))
            continue
        definitions.importLibrary(document)
        definition_paths.append(write_definitions(document, directory, os.path.splitext(file_name)[0]))

    remove_stale_definitions(directory, definition_paths)
    node_def_names = [nd.getName() for nd in definitions.getNodeDefs()]
    write_karma_node_graphs(houdini_library, definitions, node_def_names, os.path.join(directory, KARMA_FILE_NAME))
    build.node_names = write_vop_nodes(definitions, os.path.join(directory, HDA_PATH))
    return build


def compile_definitions(mxslc, source_path):
    """Returns a MaterialX document with just the NodeDefs, and their NodeGraphs, that an SLX file compiles to."""
    import MaterialX as mx
    version = compiler.choose_materialx_version(compiler.get_houdini_materialx_version(),
                                                compiler.get_available_materialx_versions(mxslc))
    options = mxslc.CompileOptions(error_on_missing_globals=False, **({"version": version} if version else {}))
    compiled = mx.createDocument()
    mx.readFromXmlString(compiled, mxslc.compile_file_to_string(source_path, options))

    document = mx.createDocument()
    for node_def in compiled.getNodeDefs():
        copy = document.addNodeDef(node_def.getName(), "", node_def.getNodeString())
        copy.copyContentFrom(node_def)
        if not copy.hasNodeGroup():
            copy.setNodeGroup(DEFAULT_NODE_GROUP)
    for graph in compiled.getNodeGraphs():
        # A graph without a NodeDef comes from a [[nodegraph]] function, which isn't a node.
        if graph.getNodeDefString():
            document.addNodeGraph(graph.getName()).copyContentFrom(graph)
    merge_template_signatures(document)
    return document


def merge_template_signatures(document):
    """Gives the NodeDefs of a templated function one node name, so they become signatures of one VOP node.

    mxslc names each instance of `T scale_by<float, vec3>(T x)` after its type: scale_by_float and scale_by_vector3.
    MaterialX's own nodes share a name across types instead, e.g. add for ND_add_float and ND_add_vector3. Only the
    node name changes; renderers find NodeDefs by their own names, which stay the same.
    """
    node_defs_by_base = {}
    for node_def in document.getNodeDefs():
        base, _, type_name = node_def.getNodeString().rpartition("_")
        if base and type_name in MATERIALX_TYPE_NAMES:
            node_defs_by_base.setdefault(base, []).append(node_def)
    for base, node_defs in node_defs_by_base.items():
        if len(node_defs) > 1:
            for node_def in node_defs:
                node_def.setNodeString(base)


def write_definitions(document, directory, name):
    import MaterialX as mx
    definitions_directory = os.path.join(directory, DEFINITIONS_DIRECTORY)
    os.makedirs(definitions_directory, exist_ok=True)
    path = os.path.join(definitions_directory, name + ".mtlx")
    mx.writeToXmlFile(document, path)
    return path


def remove_stale_definitions(directory, current_paths):
    """Removes definition files whose SLX file was deleted or no longer builds, so Houdini stops loading them."""
    current = {os.path.normcase(os.path.abspath(p)) for p in current_paths}
    for path in glob.glob(os.path.join(directory, DEFINITIONS_DIRECTORY, "*.mtlx")):
        if os.path.normcase(os.path.abspath(path)) not in current:
            os.remove(path)


def write_karma_node_graphs(houdini_library, definitions, node_def_names, path):
    import MaterialX as mx
    import mtlx2karma
    # mtlx2karma looks up the NodeDef of every node inside a graph, so it needs Houdini's libraries too.
    document = mx.createDocument()
    document.importLibrary(houdini_library)
    document.importLibrary(definitions)
    mtlx2karma.saveKarmaNodeGraphs(document, path, node_def_names)


def write_vop_nodes(definitions, path):
    """Writes an HDA with a VOP node per SLX function, installs it in this session, and returns the node type names.

    A templated function has a NodeDef per type, which become signatures of one node.
    """
    import mtlx2hda
    node_defs_by_node = {}
    for node_def in definitions.getNodeDefs():
        node_defs_by_node.setdefault(node_def.getNodeString(), []).append(node_def.getName())

    build_directory = tempfile.mkdtemp(prefix="slx_nodes_")
    try:
        build_path = os.path.join(build_directory, os.path.basename(path)).replace("\\", "/")
        for node, node_def_names in sorted(node_defs_by_node.items()):
            mtlx2hda.createHDA(NODE_TYPE_PREFIX + node, get_node_label(node), definitions, node_def_names, build_path,
                               should_append_hda=os.path.exists(build_path))
        install_vop_nodes(build_path if node_defs_by_node else None, path)
    finally:
        shutil.rmtree(build_directory, ignore_errors=True)
    return [NODE_TYPE_PREFIX + node for node in sorted(node_defs_by_node)]


def get_node_label(node):
    return "SLX " + node.replace("_", " ").title()


def install_vop_nodes(build_path, path):
    """Replaces the library's HDA with the one at build_path (None for no nodes) and updates this session."""
    installed = any(is_same_path(p, path) for p in hou.hda.loadedFiles())
    if build_path is None:
        if installed:
            hou.hda.uninstallFile(path, SESSION_ONLY_LIBRARIES, change_oplibraries_file=False)
        if os.path.exists(path):
            os.remove(path)
        return
    os.makedirs(os.path.dirname(path), exist_ok=True)
    shutil.copyfile(build_path, path)
    if installed:
        hou.hda.reloadFile(path)
    else:
        hou.hda.installFile(path, SESSION_ONLY_LIBRARIES, change_oplibraries_file=False)


def is_same_path(a, b):
    return os.path.normcase(os.path.normpath(a)) == os.path.normcase(os.path.normpath(b))


def is_on_houdini_path(directory):
    return any(is_same_path(p, directory) for p in hou.houdiniPath())


def rebuild_with_message():
    """Shelf tool: rebuilds the node library and reports what happened."""
    try:
        build = build_node_library()
    except ImportError as e:
        hou.ui.displayMessage("Can't build the SLX node library: {}".format(e), title="SLX Node Library",
                              severity=hou.severityType.Error)
        return

    lines = ["Built {} node(s) from {}.".format(len(build.node_names), build.directory)]
    if not hou.getenv("SLX_NODE_LIBRARY"):
        lines.append("SLX_NODE_LIBRARY isn't set, so Karma won't find the nodes. Update your ShadingLanguageX.json "
                     "package file from the plugin, then restart Houdini.")
    elif build.node_names:
        # Houdini only adds a package's folder to HOUDINI_PATH if it existed at startup.
        reason = "Karma in the viewport loads node definitions when it starts"
        if not is_on_houdini_path(build.directory):
            reason += ", and this folder only joins HOUDINI_PATH if it exists when Houdini starts"
        lines.append("Restart Houdini before rendering new or changed nodes in the viewport: {}. "
                     "Renders to disk and on a farm use them straight away.".format(reason))
    details = "\n\n".join("{}:\n{}".format(name, message) for name, message in build.errors) or None
    if build.errors:
        lines.append("{} file(s) couldn't be built. See the details.".format(len(build.errors)))
    hou.ui.displayMessage("\n\n".join(lines), title="SLX Node Library", details=details,
                          severity=hou.severityType.Warning if build.errors else hou.severityType.Message)


def edit_library():
    """Shelf tool: opens the node library, starting it from a template if it has no SLX files yet."""
    directory = get_library_directory()
    sources = find_library_sources(directory)
    if not sources:
        os.makedirs(directory, exist_ok=True)
        path = os.path.join(directory, "my_nodes.mxsl")
        shutil.copyfile(TEMPLATE_PATH, path)
        sources = [path]
    editor.open_in_editor(sources[0] if len(sources) == 1 else directory)
