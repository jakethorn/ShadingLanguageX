"""Finds the examples that the SLX Material node offers as a source, which live in the plugin's examples folder.

Every example names its material example_material, so switching examples keeps the node's material assignments.
Nothing in this module imports hou, so it can be tested with a plain Python interpreter.
"""

import os

from slx_houdini import compiler


def find_examples_directory():
    directory = os.path.join(compiler.PLUGIN_ROOT, "examples")
    return directory if os.path.isdir(directory) else None


def get_examples(directory):
    """Returns the paths of the examples in directory, sorted by name."""
    return [os.path.join(directory, name) for name in sorted(os.listdir(directory)) if name.endswith(".mxsl")]


def read_text(path):
    with open(path, encoding="utf-8") as f:
        return f.read()
