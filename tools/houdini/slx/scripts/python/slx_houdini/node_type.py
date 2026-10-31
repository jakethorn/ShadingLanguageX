"""Builds and installs the SLX Material LOP type.

The asset is built by the user's own Houdini at startup instead of being shipped prebuilt, for two reasons:
- an asset saved by a newer Houdini may not load in an older one, and
- an asset saved under a non-commercial or limited licence switches commercial sessions that load it into
  that licence mode.
Building it locally means the asset always matches the running Houdini and its licence. The result is cached
in the user's preferences and only rebuilt when this file, the plugin or Houdini changes.

Houdini crashes if an asset is created while pythonrc.py runs, so startup only installs an asset that is
already built. Otherwise it is built just before a scene file loads, or when an SLX tool first needs it.
"""

import contextlib
import glob
import hashlib
import os
import shutil
import tempfile

import hou

import slx_houdini
from slx_houdini import NODE_TYPE_NAME, compiler, global_parameters

LABEL = "SLX Material"
ICON = "LOP_materiallibrary"
COOK_SCRIPT = "import slx_houdini.lop\nslx_houdini.lop.cook(hou.pwd())\n"
CALLBACK = "import slx_houdini.lop; slx_houdini.lop.{}(kwargs)"
# With change_oplibraries_file=False, installing into this library list changes no file and no scene.
SESSION_ONLY_LIBRARIES = "Scanned Asset Library Directories"

# The asset's own tool has no menu contexts, which keeps it out of the Tab menu. The plugin's slx_material
# shelf tool is in the Tab menu instead, because it exists before the asset has been built.
TOOL_XML = """<?xml version="1.0" encoding="UTF-8"?>
<shelfDocument>
  <tool name="$HDA_DEFAULT_TOOL" label="$HDA_LABEL" icon="$HDA_ICON">
    <script scriptType="python"><![CDATA[import loptoolutils

loptoolutils.genericTool(kwargs, '$HDA_NAME')]]></script>
  </tool>
</shelfDocument>
"""

HELP_TEXT = """= SLX Material =

\"\"\"Compiles a ShadingLanguageX (SLX) file or inline code into MaterialX materials.\"\"\"

Each `material` variable in the SLX source becomes a material primitive under the Material Path Prefix.
The compiled MaterialX is written to the Cache Directory and referenced from there, so keep that directory
somewhere your renders can read.

Language reference: [https://github.com/jakethorn/ShadingLanguageX]
"""


def install_node_type_at_startup():
    """Installs the asset if it is already built, and makes sure the right one is installed before any scene loads.

    Houdini hasn't checked out a licence while pythonrc.py runs, so the asset installed here may have been built
    under a different licence. ensure_node_type() swaps it before a scene loads, once the licence is known.
    """
    built = glob.glob(get_definition_path("*"))
    if len(built) == 1:
        install_definition_file(built[0])
    hou.hipFile.addEventCallback(on_hip_file_event)


def on_hip_file_event(event_type):
    if event_type == hou.hipFileEventType.BeforeLoad:
        ensure_node_type()


def ensure_node_type():
    """Builds and installs the asset for this Houdini and licence, unless it's installed already.

    Not safe to call while pythonrc.py runs. The asset is installed without recording it in the scene, so saved
    scenes don't refer to a cached asset that a later plugin or Houdini version replaces.
    """
    path = get_definition_path(hou.licenseCategory().name())
    node_type = hou.nodeType(hou.lopNodeTypeCategory(), NODE_TYPE_NAME)
    installed_path = node_type.definition().libraryFilePath() if node_type else None
    if installed_path is not None and is_same_path(installed_path, path):
        return
    with pause_help_indexing():
        if not os.path.isfile(path):
            build_definition(path)
        if installed_path is not None:
            hou.hda.uninstallFile(installed_path, SESSION_ONLY_LIBRARIES, change_oplibraries_file=False)
    install_definition_file(path)
    remove_old_definitions(path)


@contextlib.contextmanager
def pause_help_indexing():
    """Stops Houdini's help from indexing assets until the block ends.

    Houdini starts a help-indexing thread for every asset event. A build makes about ten events at once, and their
    threads race and print exceptions such as an AssertionError from whoosh. The final install is indexed as usual.
    """
    callbacks = [(event_types, callback) for event_types, callback in hou.hda.eventCallbacks()
                 if getattr(callback, "__module__", "").startswith("houdinihelp")]
    for event_types, callback in callbacks:
        hou.hda.removeEventCallback(event_types, callback)
    try:
        yield
    finally:
        for event_types, callback in callbacks:
            hou.hda.addEventCallback(event_types, callback)


def install_definition_file(path):
    # Installing for the "current HIP file" would record the cached asset's path in every scene saved afterwards.
    hou.hda.installFile(path, SESSION_ONLY_LIBRARIES, change_oplibraries_file=False)


def is_same_path(a, b):
    return os.path.normcase(os.path.normpath(a)) == os.path.normcase(os.path.normpath(b))


def get_definition_path(licence_name):
    """Returns where the asset for this Houdini and licence is cached.

    The name changes with anything the asset is built from, so a changed plugin or Houdini builds a new one.
    """
    fingerprint = hashlib.sha1()
    for path in (__file__, slx_houdini.__file__, global_parameters.__file__, compiler.TEMPLATE_PATH):
        with open(os.path.abspath(path), "rb") as f:
            fingerprint.update(f.read())
    fingerprint.update("|".join([hou.applicationVersionString()] + get_materialx_versions()).encode("utf-8"))
    file_name = "slx_material_{}_{}.hda".format(fingerprint.hexdigest()[:12], licence_name.lower())
    return os.path.join(get_cache_directory(), file_name).replace("\\", "/")


def get_cache_directory():
    preferences_directory = os.path.join(hou.homeHoudiniDirectory(), "slx")
    for directory in (preferences_directory, os.path.join(hou.getenv("HOUDINI_TEMP_DIR"), "slx")):
        try:
            os.makedirs(directory, exist_ok=True)
        except OSError:
            continue
        if os.access(directory, os.W_OK):
            return directory
    raise OSError("SLX: no writable directory for the SLX Material asset.")


def get_materialx_versions():
    try:
        return compiler.get_available_materialx_versions(compiler.import_mxslc())
    except ImportError:
        return []


def build_definition(path):
    # Houdini backs the asset up each time the build changes it, so build in a folder that is deleted afterwards.
    build_directory = tempfile.mkdtemp(prefix="slx_build_")
    build_path = os.path.join(build_directory, os.path.basename(path)).replace("\\", "/")
    try:
        with hou.undos.disabler():
            network = hou.node("/obj").createNode("lopnet", "slx_build")
            try:
                subnet = network.createNode("subnet", "slx_material")
                script = subnet.createNode("pythonscript", "cook")
                script.setInput(0, subnet.indirectInputs()[0])
                script.parm("python").set(COOK_SCRIPT)
                subnet.node("output0").setInput(0, script)
                asset = subnet.createDigitalAsset(name=NODE_TYPE_NAME, hda_file_name=build_path, description=LABEL,
                                                  min_num_inputs=0, max_num_inputs=1, ignore_external_references=True)
                definition = asset.type().definition()
                definition.setParmTemplateGroup(create_parm_template_group())
                definition.setIcon(ICON)
                definition.addSection("Tools.shelf", TOOL_XML)
                definition.addSection("Help", HELP_TEXT)
                definition.setVersion(slx_houdini.__version__)
                definition.save(build_path)
            finally:
                network.destroy()
        hou.hda.uninstallFile(build_path)
        move_into_place(build_path, path)
    finally:
        shutil.rmtree(build_directory, ignore_errors=True)


def move_into_place(source, path):
    # Several Houdini processes can start at once on a render farm, so each copies to its own temporary file first.
    temp_path = "{}.{}.tmp".format(path, os.getpid())
    shutil.copyfile(source, temp_path)
    try:
        os.replace(temp_path, path)
    except OSError:
        # Another process won the race and its file is in use; that file is identical, so use it.
        os.remove(temp_path)


def remove_old_definitions(current_path):
    for path in glob.glob(os.path.join(os.path.dirname(current_path), "slx_material_*.hda")):
        if not is_same_path(path, current_path):
            try:
                os.remove(path)
            except OSError:
                pass


def create_parm_template_group():
    group = hou.ParmTemplateGroup()
    group.append(create_source_folder())
    group.append(create_globals_folder())
    group.append(create_materials_folder())
    group.append(create_compile_options_folder())
    group.append(create_output_folder())
    return group


def create_button(name, label, callback, join_with_next=False, conditionals=None):
    return hou.ButtonParmTemplate(name, label, join_with_next=join_with_next, script_callback=CALLBACK.format(callback),
                                  script_callback_language=hou.scriptLanguage.Python, conditionals=conditionals or {})


def create_source_folder():
    hide_unless_file = {hou.parmCondType.HideWhen: "{ source != file }"}
    hide_unless_inline = {hou.parmCondType.HideWhen: "{ source != inline }"}
    return hou.FolderParmTemplate("source_folder", "Source", folder_type=hou.folderType.Tabs, parm_templates=[
        hou.MenuParmTemplate("source", "Source", menu_items=("file", "inline"), menu_labels=("File", "Inline Code"),
                             help="Compile an SLX file, or SLX code typed into this node."),
        hou.StringParmTemplate("file", "SLX File", 1, string_type=hou.stringParmType.FileReference,
                               file_type=hou.fileType.Any, conditionals=hide_unless_file,
                               tags={"filechooser_pattern": "*.mxsl", "filechooser_mode": "read"},
                               help="The .mxsl file to compile. Its #include and #library paths, and other "
                                    "relative paths, are resolved from its folder."),
        hou.StringParmTemplate("code", "Code", 1, default_value=(compiler.read_template(),),
                               conditionals=hide_unless_inline,
                               tags={"editor": "1", "editorlines": "20-60"},
                               help="SLX code to compile. Its #include and #library paths are resolved from $HIP."),
        create_button("compile", "Compile", "on_compile_pressed", join_with_next=True),
        create_button("openeditor", "Open in Editor", "on_open_editor_pressed", join_with_next=True,
                      conditionals=hide_unless_file),
        create_button("createfile", "Create File", "on_create_file_pressed", join_with_next=True,
                      conditionals=hide_unless_file),
        create_button("loadexample", "Load Example...", "on_load_example_pressed"),
        hou.ToggleParmTemplate("autorecompile", "Recompile When Files Change", default_value=True,
                               conditionals=hide_unless_file,
                               help="Recompile whenever the SLX file, or a file it includes, is saved."),
        hou.IntParmTemplate("recompile", "Recompile", 1, is_hidden=True),
    ])


def create_globals_folder():
    # Cooks fill this tab with a spare parameter for each global variable (see slx_houdini.global_parameters).
    return hou.FolderParmTemplate(global_parameters.FOLDER_NAME, global_parameters.FOLDER_LABEL,
                                  folder_type=hou.folderType.Tabs, parm_templates=[
        hou.LabelParmTemplate(global_parameters.NO_GLOBALS_LABEL_NAME, "",
                              column_labels=("The SLX source doesn't declare any global variables.",)),
    ])


def create_materials_folder():
    menu_script = "import slx_houdini.lop\nreturn slx_houdini.lop.get_material_menu_items(kwargs['node'])"
    select_prims = "import loputils\nloputils.selectPrimsInParm(kwargs, True)"
    return hou.FolderParmTemplate("materials_folder", "Materials", folder_type=hou.folderType.Tabs, parm_templates=[
        hou.StringParmTemplate("matpathprefix", "Material Path Prefix", 1, default_value=("/materials/",),
                               help="Each material in the SLX source is created at this path plus its variable name."),
        hou.FolderParmTemplate("assignments", "Assignments", folder_type=hou.folderType.MultiparmBlock, parm_templates=[
            hou.StringParmTemplate("material#", "Material", 1, menu_type=hou.menuType.StringReplace,
                                   item_generator_script=menu_script,
                                   item_generator_script_language=hou.scriptLanguage.Python,
                                   help="The name of a material variable in the SLX source."),
            hou.StringParmTemplate("geometry#", "Geometry", 1, help="The primitives to bind the material to.",
                                   tags={"script_action": select_prims, "script_action_icon": "BUTTONS_reselect",
                                         "script_action_help": "Select primitives in the viewer or scene graph tree.",
                                         "sidefx::usdpathtype": "primlist"}),
        ]),
    ])


def create_compile_options_folder():
    houdini_version = compiler.get_houdini_materialx_version() or "unknown"
    versions = get_materialx_versions()
    return hou.FolderParmTemplate("options_folder", "Compile Options", folder_type=hou.folderType.Tabs, parm_templates=[
        hou.StringParmTemplate("materialxversion", "MaterialX Version", 1, default_value=("auto",),
                               menu_items=["auto"] + versions,
                               menu_labels=["Match Houdini ({})".format(houdini_version)] + versions,
                               menu_type=hou.menuType.Normal,
                               help="The MaterialX library version to compile against."),
        hou.StringParmTemplate("entryfunction", "Entry Function", 1,
                               help="A function to call after the file has run, like mxslc --func."),
        hou.StringParmTemplate("entryarguments", "Entry Arguments", 1,
                               conditionals={hou.parmCondType.DisableWhen: '{ entryfunction == "" }'},
                               help='Comma separated arguments for the entry function, '
                                    'e.g. 0.5, "brick.png", (1, 0, 0).'),
        hou.FolderParmTemplate("macros", "Macros", folder_type=hou.folderType.MultiparmBlock, parm_templates=[
            hou.StringParmTemplate("macroname#", "Name", 1, join_with_next=True),
            hou.StringParmTemplate("macrovalue#", "Value", 1, help="Leave empty to define a flag macro."),
        ]),
        hou.ToggleParmTemplate("reducegraph", "Reduce Graph", default_value=True,
                               help="Fold compile-time values into constants."),
        hou.ToggleParmTemplate("inlinefunctions", "Inline SLX Functions", default_value=True,
                               help="Mark all SLX functions as inline, bypassing MaterialX NodeDef/NodeGraph "
                                    "elements."),
    ])


def create_output_folder():
    return hou.FolderParmTemplate("output_folder", "Output", folder_type=hou.folderType.Tabs, parm_templates=[
        hou.StringParmTemplate("cachedir", "Cache Directory", 1, default_value=("$HIP/slx_cache",),
                               string_type=hou.stringParmType.FileReference, file_type=hou.fileType.Directory,
                               help="Where compiled .mtlx files are written. Renders read them from here."),
        create_button("viewmaterialx", "View MaterialX", "on_view_materialx_pressed", join_with_next=True),
        create_button("exportmaterialx", "Export MaterialX...", "on_export_materialx_pressed"),
    ])
