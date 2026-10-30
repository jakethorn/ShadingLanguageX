"""Opens SLX source files in the user's text editor."""

import os
import shlex
import shutil
import subprocess
import sys

import hou


def open_in_editor(path, line=None):
    """Opens path in $SLX_EDITOR, $VISUAL or $EDITOR, then VS Code, then the OS default application."""
    editor = hou.getenv("SLX_EDITOR") or hou.getenv("VISUAL") or hou.getenv("EDITOR")
    if editor:
        subprocess.Popen(shlex.split(editor, posix=sys.platform != "win32") + [path])
        return

    code = shutil.which("code")
    if code:
        target = "{}:{}".format(path, line) if line else path
        subprocess.Popen([code, "--goto", target])
        return

    if sys.platform == "win32":
        os.startfile(path)
    elif sys.platform == "darwin":
        subprocess.Popen(["open", path])
    else:
        subprocess.Popen(["xdg-open", path])
