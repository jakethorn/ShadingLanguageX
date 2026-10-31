"""Tests for the parts of the plugin that don't need Houdini. Run them with any Python 3.9+:

    python tests/test_compiler.py
"""

import os
import shutil
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "slx", "scripts", "python"))

from slx_houdini import compiler, examples  # noqa: E402

AVAILABLE = ["1.38.10", "1.39.4", "1.39.5"]


class ChooseMaterialXVersionTests(unittest.TestCase):
    def test_exact_match(self):
        self.assertEqual(compiler.choose_materialx_version("1.39.5", AVAILABLE), "1.39.5")

    def test_newest_version_not_newer_than_houdini(self):
        self.assertEqual(compiler.choose_materialx_version("1.39.4", AVAILABLE), "1.39.4")
        self.assertEqual(compiler.choose_materialx_version("1.39.9", AVAILABLE), "1.39.5")

    def test_only_newer_versions_of_the_same_minor(self):
        self.assertEqual(compiler.choose_materialx_version("1.38.7", AVAILABLE), "1.38.10")
        self.assertEqual(compiler.choose_materialx_version("1.39.3", AVAILABLE), "1.39.4")

    def test_version_without_patch_uses_newest_of_its_minor(self):
        self.assertEqual(compiler.choose_materialx_version("1.39", AVAILABLE), "1.39.5")

    def test_unknown_houdini_version_uses_newest(self):
        self.assertEqual(compiler.choose_materialx_version(None, AVAILABLE), "1.39.5")


class ParseValueTests(unittest.TestCase):
    def test_literals(self):
        self.assertEqual(compiler.parse_value("0.5"), 0.5)
        self.assertEqual(compiler.parse_value("2"), 2)
        self.assertIs(compiler.parse_value("true"), True)
        self.assertEqual(compiler.parse_value('"a.png"'), "a.png")
        self.assertEqual(compiler.parse_value("(1, 0.5, 0)"), [1, 0.5, 0])

    def test_text_that_isnt_a_literal_is_a_string(self):
        self.assertEqual(compiler.parse_value("textures/brick.png"), "textures/brick.png")

    def test_value_list_splits_on_top_level_commas(self):
        self.assertEqual(compiler.parse_value_list('0.5, "a, b.png", (1, 0, 0)'), [0.5, "a, b.png", [1, 0, 0]])
        self.assertEqual(compiler.parse_value_list(""), [])


class FindSourceFilesTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.mkdtemp()

    def tearDown(self):
        shutil.rmtree(self.directory)

    def write_file(self, name, text):
        path = os.path.join(self.directory, name)
        os.makedirs(os.path.dirname(path), exist_ok=True)
        with open(path, "w") as f:
            f.write(text)
        return path

    def test_follows_includes_and_libraries(self):
        main = self.write_file("main.mxsl", '#include "lib/a.mxsl"\n#library "defs.mtlx"\n')
        self.write_file("lib/a.mxsl", '#include "b.mxsl"\n  #  include "missing.mxsl"\n')
        self.write_file("lib/b.mxsl", '#include "../main.mxsl"\n')
        found = [os.path.relpath(p, self.directory) for p in compiler.find_source_files(main)]
        expected = ["main.mxsl", "lib/a.mxsl", "defs.mtlx", "lib/b.mxsl", "lib/missing.mxsl"]
        self.assertEqual(found, [os.path.normpath(p) for p in expected])

    def test_library_paths_are_relative_to_the_main_file(self):
        main = self.write_file("main.mxsl", '#include "lib/a.mxsl"\n')
        self.write_file("lib/a.mxsl", '#library "defs.mtlx"\n')
        self.assertIn(os.path.join(self.directory, "defs.mtlx"), compiler.find_source_files(main))


class ExampleTests(unittest.TestCase):
    def test_examples_make_example_material_without_textures_or_displacement(self):
        paths = examples.get_examples(examples.find_examples_directory())
        self.assertTrue(paths)
        for path in paths:
            text = examples.read_text(path)
            self.assertIn("material example_material = surfacematerial(", text, path)
            self.assertNotRegex(text, r"\bimage\s*\(|\.(png|jpe?g|exr|hdr|tiff?|tx)\b", path)
            # Karma CPU in Houdini 22.0 crashes when its viewport switches from a displaced material to one without
            # displacement, as it does when switching examples.
            self.assertNotIn("displacement", text, path)


if __name__ == "__main__":
    unittest.main()
