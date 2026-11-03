"""Installs the plugin's asset files into the running Houdini session only.

Installing for the "current HIP file" would record the asset's path in every scene saved afterwards, and installing
into an oplibraries file would change a file on disk. Neither happens with the library list used here.
"""

import os

import hou

# With change_oplibraries_file=False, installing into this library list changes no file and no scene.
SESSION_ONLY_LIBRARIES = "Scanned Asset Library Directories"


def install_for_session(path):
    hou.hda.installFile(path, SESSION_ONLY_LIBRARIES, change_oplibraries_file=False)


def uninstall_for_session(path):
    hou.hda.uninstallFile(path, SESSION_ONLY_LIBRARIES, change_oplibraries_file=False)


def is_installed(path):
    return any(is_same_path(p, path) for p in hou.hda.loadedFiles())


def is_same_path(a, b):
    return os.path.normcase(os.path.normpath(a)) == os.path.normcase(os.path.normpath(b))
