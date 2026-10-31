"""Finds the ShadingLanguageX examples and copies them next to the scene, where they can be edited.

Nothing in this module imports hou, so it can be tested with a plain Python interpreter.
"""

import os
import re
import shutil

from slx_houdini import compiler

MATERIAL_PATTERN = re.compile(r"\b(?:surface|volume)material[ \t]*\(|^[ \t]*material[ \t]+\w+", re.MULTILINE)
IMAGE_PATTERN = re.compile(r'"([^"\n]+\.(?:png|jpe?g|exr|hdr|tiff?|tx))"', re.IGNORECASE)


def find_examples_directory():
    """Returns the examples folder: slx/examples in a release, or the repository's examples folder in a checkout."""
    for directory in (os.path.join(compiler.PLUGIN_ROOT, "examples"),
                      os.path.join(compiler.PLUGIN_ROOT, "..", "..", "..", "examples")):
        if os.path.isdir(directory):
            return os.path.normpath(directory)
    return None


def get_examples(directory):
    """Returns the paths of the examples in directory that create a material, sorted by name."""
    examples = []
    for name in sorted(os.listdir(directory)):
        path = os.path.join(directory, name)
        if name.endswith(".mxsl") and MATERIAL_PATTERN.search(read_text(path)):
            examples.append(path)
    return examples


def copy_example(path, destination_directory):
    """Copies an example, the files it includes and the images it uses into destination_directory.

    Files already there are kept, so changes to an example loaded before survive. Returns the copied example's path.
    """
    example_directory = os.path.dirname(os.path.abspath(path))
    for source_path in find_example_files(path):
        relative_path = os.path.relpath(source_path, example_directory)
        destination_path = os.path.join(destination_directory, relative_path)
        if relative_path.startswith("..") or os.path.exists(destination_path):
            continue
        os.makedirs(os.path.dirname(destination_path), exist_ok=True)
        shutil.copyfile(source_path, destination_path)
    return os.path.join(destination_directory, os.path.basename(path))


def find_example_files(path):
    """Returns the example's file, the files it includes, and the images its source files name."""
    example_directory = os.path.dirname(os.path.abspath(path))
    files = [p for p in compiler.find_source_files(path) if os.path.isfile(p)]
    for source_path in [p for p in files if p.endswith(".mxsl")]:
        for image in IMAGE_PATTERN.findall(read_text(source_path)):
            # Examples write image paths relative to their folder, sometimes with backslashes.
            image_path = os.path.normpath(os.path.join(example_directory, image.replace("\\", "/")))
            if os.path.isfile(image_path) and image_path not in files:
                files.append(image_path)
    return files


def read_text(path):
    with open(path, encoding="utf-8") as f:
        return f.read()
