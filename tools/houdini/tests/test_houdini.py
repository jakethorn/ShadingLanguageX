"""Integration tests for the SLX Houdini plugin. Run them with hython, with the plugin on HOUDINI_PATH:

    HOUDINI_PATH="/path/to/tools/houdini/slx;&" hython tests/test_houdini.py

To keep the built asset out of your Houdini preferences, set HOUDINI_USER_PREF_DIR to a temporary folder
whose name contains __HVER__ (Houdini ignores it otherwise), e.g. /tmp/slx_prefs/houdini__HVER__.
"""

import os
import shutil
import subprocess
import sys
import tempfile
import unittest

import hou
import voptoolutils
from pxr import UsdShade

import slx_houdini.convert
import slx_houdini.examples
import slx_houdini.lop
import slx_houdini.node_library
import slx_houdini.node_type
from slx_houdini import NODE_TYPE_NAME

MAIN_SOURCE = """
#include "utils.mxsl"
surfaceshader surface = standard_surface(base = add_one(0.2));
material {} = surfacematerial(surface);
"""
EXAMPLES_DIRECTORY = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", "examples")
UTILS_SOURCE = "float add_one(float x) { return x + 1.0; }\n"
GLOBALS_SOURCE = """
global float roughness = {roughness};
global color3 tint = color3{{1, 0.5, 0}};
global vec2 offset;
global int count = 3;
global bool shiny = true;
global string texture = "brick.png";
global float u = texcoord<vec2>().x;
material mat = surfacematerial(standard_surface(base_color = tint, specular_roughness = roughness));
"""


class SlxMaterialTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        slx_houdini.node_type.ensure_node_type()

    def setUp(self):
        self.directory = tempfile.mkdtemp(prefix="slx_test_").replace("\\", "/")
        self.network = hou.node("/stage")
        self.node = self.network.createNode(NODE_TYPE_NAME)
        self.node.parm("source").set("inline")
        self.node.parm("cachedir").set(self.directory + "/cache")

    def tearDown(self):
        for node in self.network.children():
            node.destroy()
        shutil.rmtree(self.directory, ignore_errors=True)

    def write_file(self, name, text):
        path = "{}/{}".format(self.directory, name)
        with open(path, "w") as f:
            f.write(text)
        return path

    def use_file_source(self, material_name="mat"):
        self.write_file("utils.mxsl", UTILS_SOURCE)
        path = self.write_file("main.mxsl", MAIN_SOURCE.format(material_name))
        self.node.parm("source").set("file")
        self.node.parm("file").set(path)
        return path

    def get_material(self, path):
        stage = self.node.stage()
        self.assertIsNotNone(stage, self.node.errors())
        return UsdShade.Material(stage.GetPrimAtPath(path))

    def test_node_type_is_built_in_preferences(self):
        definition = hou.nodeType(hou.lopNodeTypeCategory(), NODE_TYPE_NAME).definition()
        self.assertTrue(definition.libraryFilePath().startswith(hou.homeHoudiniDirectory().replace("\\", "/")))

    def test_inline_template_creates_material(self):
        material = self.get_material("/materials/slx_material")
        surface = material.ComputeSurfaceSource("mtlx")[0]
        self.assertEqual(surface.GetIdAttr().Get(), "ND_standard_surface_surfaceshader")

    def get_custom_nodes(self):
        stage = self.get_material("/materials/mat").GetPrim().GetStage()
        return [p for p in stage.Traverse() if p.GetAttribute("info:mtlx:sourceAsset").Get() is not None]

    def test_file_source_watches_included_files(self):
        path = self.use_file_source()
        self.assertTrue(self.get_material("/materials/mat"))
        self.assertIn(os.path.normpath(self.directory + "/utils.mxsl"), slx_houdini.compiler.find_source_files(path))

    def test_function_nodes_are_expanded_for_karma(self):
        self.use_file_source()
        self.assertEqual(self.get_custom_nodes(), [])
        self.node.parm("expandfunctions").set(False)
        custom_nodes = self.get_custom_nodes()
        self.assertEqual(len(custom_nodes), 1)
        self.assertTrue(os.path.isfile(custom_nodes[0].GetAttribute("info:mtlx:sourceAsset").Get().resolvedPath))

    def get_errors(self):
        try:
            self.node.cook(force=True)
        except hou.OperationFailed:
            pass
        return " ".join(self.node.errors())

    def test_relative_file_paths_are_relative_to_the_source_file(self):
        self.write_file("wood.png", "")
        path = self.write_file("main.mxsl", 'color3 c = image("wood.png");\n'
                                            "material mat = surfacematerial(standard_surface(base_color = c));\n")
        self.node.parm("source").set("file")
        self.node.parm("file").set(path)
        stage = self.get_material("/materials/mat").GetPrim().GetStage()
        files = [p.GetAttribute("inputs:file").Get() for p in stage.Traverse() if p.HasAttribute("inputs:file")]
        expected_path = os.path.normcase(os.path.normpath(os.path.join(os.path.dirname(path), "wood.png")))
        self.assertEqual([os.path.normcase(f.resolvedPath) for f in files], [expected_path])

    def test_example_compiles_after_copying(self):
        example = os.path.join(EXAMPLES_DIRECTORY, "redbrick.mxsl")
        copied = slx_houdini.examples.copy_example(example, self.directory + "/slx_examples")
        self.node.parm("source").set("file")
        self.node.parm("file").set(copied)
        self.assertTrue(self.get_material("/materials/brick_material"))

    def test_compile_error_is_shown_on_node(self):
        self.node.parm("code").set("float x = 1.0\nfloat y = 2.0;")
        errors = self.get_errors()
        self.assertIn("line 2", errors)
        self.assertEqual(slx_houdini.lop._compile_results[self.node.sessionId()].error_line(), 2)

    def test_missing_file_is_reported(self):
        self.node.parm("source").set("file")
        self.node.parm("file").set(self.directory + "/missing.mxsl")
        self.assertIn("SLX file not found", self.get_errors())

    def test_empty_file_path_passes_input_through(self):
        self.node.parm("source").set("file")
        self.assertEqual(self.get_errors(), "")
        self.assertFalse(self.node.stage().GetPrimAtPath("/materials"))

    def test_source_without_materials_is_an_error(self):
        self.node.parm("code").set("float x = 1.0;")
        self.assertIn("doesn't define any materials", self.get_errors())

    def test_material_is_assigned_to_geometry(self):
        sphere = self.network.createNode("sphere")
        self.node.setInput(0, sphere)
        self.node.parm("assignments").set(1)
        self.node.parm("material1").set("slx_material")
        self.node.parm("geometry1").set("/sphere1")
        prim = self.node.stage().GetPrimAtPath("/sphere1")
        bound = UsdShade.MaterialBindingAPI(prim).ComputeBoundMaterial()[0]
        self.assertEqual(str(bound.GetPath()), "/materials/slx_material")

    def test_recompile_reads_changed_file(self):
        path = self.use_file_source("before")
        self.assertTrue(self.get_material("/materials/before"))
        self.write_file("main.mxsl", MAIN_SOURCE.format("after"))
        slx_houdini.lop.recompile(self.node)
        self.assertTrue(self.get_material("/materials/after"))
        self.assertFalse(self.get_material("/materials/before"))

    def test_macros(self):
        self.node.parm("code").set(
            "#ifdef ROUGH\nfloat r = 0.9;\n#else\nfloat r = 0.1;\n#endif\n"
            "material mat = surfacematerial(standard_surface(specular_roughness = r));\n"
        )
        self.node.parm("macros").set(1)
        self.node.parm("macroname1").set("ROUGH")
        surface = self.get_material("/materials/mat").ComputeSurfaceSource("mtlx")[0]
        self.assertAlmostEqual(get_input_value(surface, "specular_roughness"), 0.9, places=5)

    def set_code_and_update_globals(self, code):
        self.node.parm("code").set(code)
        slx_houdini.lop.update_global_parameters(self.node)

    def test_globals_become_parameters(self):
        self.set_code_and_update_globals(GLOBALS_SOURCE.format(roughness=0.25))
        parm_types = {t.name(): (t.type(), t.numComponents(), t.look())
                      for t in self.node.parmTemplateGroup().findFolder("Globals").parmTemplates()
                      if t.name().startswith("global_")}
        self.assertEqual(parm_types, {
            "global_roughness": (hou.parmTemplateType.Float, 1, hou.parmLook.Regular),
            "global_tint": (hou.parmTemplateType.Float, 3, hou.parmLook.ColorSquare),
            "global_offset": (hou.parmTemplateType.Float, 2, hou.parmLook.Regular),
            "global_count": (hou.parmTemplateType.Int, 1, hou.parmLook.Regular),
            "global_shiny": (hou.parmTemplateType.Toggle, 1, hou.parmLook.Regular),
            "global_texture": (hou.parmTemplateType.String, 1, hou.parmLook.Regular),
            "global_u": (hou.parmTemplateType.Label, 1, hou.parmLook.Regular),
        })
        self.assertEqual(self.node.parmTuple("global_tint").eval(), (1.0, 0.5, 0.0))
        self.assertTrue(self.node.parmTemplateGroup().find("noglobals").isHidden())

    def test_global_parameters_change_the_material(self):
        self.set_code_and_update_globals(GLOBALS_SOURCE.format(roughness=0.25))
        self.node.parmTuple("global_tint").set((0, 0, 1))
        self.node.parm("global_roughness").set(0.75)
        surface = self.get_material("/materials/mat").ComputeSurfaceSource("mtlx")[0]
        self.assertEqual(tuple(get_input_value(surface, "base_color")), (0, 0, 1))
        self.assertAlmostEqual(get_input_value(surface, "specular_roughness"), 0.75, places=5)

    def test_changed_initializers_only_move_untouched_parameters(self):
        self.set_code_and_update_globals(GLOBALS_SOURCE.format(roughness=0.25))
        self.node.parmTuple("global_tint").set((0, 0, 1))
        self.set_code_and_update_globals(GLOBALS_SOURCE.format(roughness=0.5).replace("1, 0.5, 0", "0, 1, 0"))
        self.assertAlmostEqual(self.node.parm("global_roughness").eval(), 0.5)
        self.assertEqual(self.node.parmTuple("global_tint").eval(), (0.0, 0.0, 1.0))

    def test_removed_globals_lose_their_parameters(self):
        self.set_code_and_update_globals(GLOBALS_SOURCE.format(roughness=0.25))
        self.set_code_and_update_globals("material mat = surfacematerial(standard_surface());")
        self.assertIsNone(self.node.parmTuple("global_roughness"))
        self.assertFalse(self.node.parmTemplateGroup().find("noglobals").isHidden())

    def test_convert_karma_material_builder_to_slx(self):
        library = self.network.createNode("materiallibrary")
        builder = voptoolutils._setupMtlXBuilderSubnet(destination_node=library, name="wood",
                                                       mask=voptoolutils.KARMAMTLX_TAB_MASK,
                                                       folder_label="Karma Material Builder", render_context="kma")
        image = builder.createNode("mtlximage")
        image.parm("file").set("wood.png")
        surface = builder.node("mtlxstandard_surface")
        surface.setNamedInput("base_color", image, "out")
        surface.parm("specular_roughness").set(0.75)
        self.assertTrue(slx_houdini.convert.can_convert(image))

        converted = slx_houdini.convert.convert_to_slx(image)[0]
        converted.parm("cachedir").set(self.directory + "/cache")
        stage = converted.stage()
        self.assertIsNotNone(stage, (converted.parm("code").unexpandedString(), converted.errors()))
        material = UsdShade.Material(stage.GetPrimAtPath("/materials/wood"))
        shader = material.ComputeSurfaceSource("mtlx")[0]
        self.assertAlmostEqual(get_input_value(shader, "specular_roughness"), 0.75, places=5)
        image_shader = shader.GetInput("base_color").GetConnectedSources()[0][0].source
        image_path = get_input_value(UsdShade.Shader(image_shader.GetPrim()), "file").path
        self.assertEqual(image_path, hou.text.expandString("$HIP/wood.png"))  # inline code is relative to $HIP
        self.assertTrue(material.ComputeDisplacementSource("mtlx")[0])


LIBRARY_SOURCE = """
@doc "Repeating ramps from 0 to 1."
float stripes(float x, float count = 8.0) { return fract(x * count); }
T scale_by<float, vec3>(T x, float s = 2.0) { return x * s; }
inline float helper(float x) { return x; }
"""


class NodeLibraryTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.mkdtemp(prefix="slx_library_").replace("\\", "/")

    def tearDown(self):
        for node in hou.node("/stage").children():
            node.destroy()
        hda_path = os.path.join(self.directory, slx_houdini.node_library.HDA_PATH)
        if any(slx_houdini.node_library.is_same_path(p, hda_path) for p in hou.hda.loadedFiles()):
            hou.hda.uninstallFile(hda_path, slx_houdini.node_library.SESSION_ONLY_LIBRARIES,
                                  change_oplibraries_file=False)
        shutil.rmtree(self.directory, ignore_errors=True)

    def write_source(self, name, text):
        with open(os.path.join(self.directory, name), "w") as f:
            f.write(text)

    def test_functions_become_vop_nodes(self):
        self.write_source("nodes.mxsl", LIBRARY_SOURCE)
        self.write_source("broken.mxsl", "float f(float x) { return x }")
        build = slx_houdini.node_library.build_node_library(self.directory)

        self.assertEqual(build.node_names, ["slx_scale_by", "slx_stripes"])
        self.assertEqual([name for name, _ in build.errors], ["broken.mxsl"])
        self.assertEqual(hou.nodeType(hou.vopNodeTypeCategory(), "slx_stripes").description(), "SLX Stripes")
        # A templated function is one node, with a signature per type.
        scale_by = hou.node("/mat").createNode("slx_scale_by")
        self.assertEqual(scale_by.shaderNameForSignature("byvector3"), "ND_scale_by_vector3")
        scale_by.parm("signature").set("byvector3")
        self.assertEqual(scale_by.inputDataTypes()[0], "vector")  # Houdini's name for vector3
        scale_by.destroy()
        with open(os.path.join(self.directory, slx_houdini.node_library.KARMA_FILE_NAME)) as f:
            karma_definitions = f.read()
        for name in ("ND_stripes", "ND_scale_by_float", "ND_scale_by_vector3"):
            self.assertIn(name, karma_definitions)
        self.assertTrue(os.path.isfile(os.path.join(self.directory, "materialx", "libraries", "slx", "nodes.mtlx")))

    def test_function_defined_twice_is_reported(self):
        self.write_source("a.mxsl", LIBRARY_SOURCE)
        self.write_source("b.mxsl", "float stripes(float x) { return x; }")
        build = slx_houdini.node_library.build_node_library(self.directory)
        self.assertEqual(build.errors[0][0], "b.mxsl")
        self.assertIn("ND_stripes", build.errors[0][1])

    def test_library_nodes_render_in_karma(self):
        self.write_source("nodes.mxsl", LIBRARY_SOURCE)
        slx_houdini.node_library.build_node_library(self.directory)
        stage = hou.node("/stage")
        sphere = stage.createNode("sphere")
        library = stage.createNode("materiallibrary")
        library.setInput(0, sphere)
        builder = voptoolutils._setupMtlXBuilderSubnet(destination_node=library, name="striped",
                                                       mask=voptoolutils.KARMAMTLX_TAB_MASK,
                                                       folder_label="Karma Material Builder", render_context="kma")
        builder.node("mtlxstandard_surface").setNamedInput("base", builder.createNode("slx_stripes"), 0)
        assign = stage.createNode("assignmaterial")
        assign.setInput(0, library)
        assign.parm("primpattern1").set("/sphere1")
        assign.parm("matspecpath1").set("/materials/striped")
        camera = stage.createNode("camera")
        camera.setInput(0, assign)
        camera.parmTuple("t").set((0, 0, 5))
        usd_path = os.path.join(self.directory, "scene.usda")
        camera.stage().Export(usd_path)

        # husk is a new process, so it finds the library's karmaNodeGraphs.json the way a farm render would.
        husk = os.path.join(hou.getenv("HFS"), "bin", "husk.exe" if sys.platform == "win32" else "husk")
        environment = dict(os.environ, HOUDINI_PATH=self.directory + os.pathsep + "&")
        result = subprocess.run([husk, "--renderer", "BRAY_HdKarma", "--res", "64", "48", "-f", "1",
                                 "-o", os.path.join(self.directory, "render.png"), usd_path],
                                env=environment, capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertNotIn("Unhandled node type", result.stdout + result.stderr)
        self.assertTrue(os.path.isfile(os.path.join(self.directory, "render.png")))


def get_input_value(shader, name):
    """Returns an input's value, following connections to the material's interface inputs."""
    return shader.GetInput(name).GetValueProducingAttributes()[0].Get()


if __name__ == "__main__":
    result = unittest.main(argv=sys.argv[:1], exit=False, verbosity=2).result
    sys.exit(0 if result.wasSuccessful() else 1)
