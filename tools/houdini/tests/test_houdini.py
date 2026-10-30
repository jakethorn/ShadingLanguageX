"""Integration tests for the SLX Houdini plugin. Run them with hython, with the plugin on HOUDINI_PATH:

    HOUDINI_PATH="/path/to/tools/houdini/slx;&" hython tests/test_houdini.py

To keep the built asset out of your Houdini preferences, set HOUDINI_USER_PREF_DIR to a temporary folder
whose name contains __HVER__ (Houdini ignores it otherwise), e.g. /tmp/slx_prefs/houdini__HVER__.
"""

import os
import shutil
import sys
import tempfile
import unittest

import hou
import voptoolutils
from pxr import UsdShade

import slx_houdini.convert
import slx_houdini.lop
import slx_houdini.node_type
from slx_houdini import NODE_TYPE_NAME

MAIN_SOURCE = """
#include "utils.mxsl"
surfaceshader surface = standard_surface(base = add_one(0.2));
material {} = surfacematerial(surface);
"""
UTILS_SOURCE = "float add_one(float x) { return x + 1.0; }\n"


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

    def test_globals_and_macros(self):
        self.node.parm("code").set(
            "global color3 tint;\n"
            "#ifdef ROUGH\nfloat r = 0.9;\n#else\nfloat r = 0.1;\n#endif\n"
            "material mat = surfacematerial(standard_surface(base_color = tint, specular_roughness = r));\n"
        )
        self.node.parm("globals").set(1)
        self.node.parm("globalname1").set("tint")
        self.node.parm("globalvalue1").set("(1, 0.5, 0)")
        self.node.parm("macros").set(1)
        self.node.parm("macroname1").set("ROUGH")
        surface = self.get_material("/materials/mat").ComputeSurfaceSource("mtlx")[0]
        self.assertAlmostEqual(get_input_value(surface, "specular_roughness"), 0.9, places=5)
        self.assertEqual(tuple(get_input_value(surface, "base_color")), (1, 0.5, 0))

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
        self.assertEqual(get_input_value(UsdShade.Shader(image_shader.GetPrim()), "file").path, "wood.png")
        self.assertTrue(material.ComputeDisplacementSource("mtlx")[0])


def get_input_value(shader, name):
    """Returns an input's value, following connections to the material's interface inputs."""
    return shader.GetInput(name).GetValueProducingAttributes()[0].Get()


if __name__ == "__main__":
    result = unittest.main(argv=sys.argv[:1], exit=False, verbosity=2).result
    sys.exit(0 if result.wasSuccessful() else 1)
