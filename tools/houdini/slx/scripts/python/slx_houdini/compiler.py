"""Finds and configures the mxslc compiler for the running Houdini.

Nothing in this module imports hou, so it can be tested with a plain Python interpreter.
"""

import ast
import os
import re
import sys

PLUGIN_ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
TEMPLATE_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), "templates", "material.mxsl")

INCLUDE_PATTERN = re.compile(r'^[ \t]*#[ \t]*(include|library)[ \t]+"([^"]+)"', re.MULTILINE)
VERSION_PATTERN = re.compile(r"^\d+\.\d+\.\d+$")


def get_vendor_directory():
    """Returns the folder holding the mxslc build for this OS and Python version."""
    os_name = {"win32": "windows", "darwin": "macos"}.get(sys.platform, "linux")
    return os.path.join(PLUGIN_ROOT, "vendor", "{}-cp{}{}".format(os_name, *sys.version_info[:2]))


def import_mxslc():
    """Imports mxslc, preferring the copy vendored with the plugin over any installed one."""
    vendor_directory = get_vendor_directory()
    if os.path.isdir(vendor_directory) and vendor_directory not in sys.path:
        sys.path.insert(0, vendor_directory)
    try:
        import mxslc
    except ImportError as e:
        raise ImportError(
            "Could not import mxslc ({}). Run tools/houdini/vendor_mxslc.py to download it into {}, "
            "or pip install mxslcxx into Houdini's Python.".format(e, vendor_directory)
        ) from e
    return mxslc


def get_available_materialx_versions(mxslc):
    """Returns the MaterialX library versions mxslc can compile for, oldest first."""
    libraries = os.path.join(os.path.dirname(mxslc.__file__), "libraries")
    try:
        names = os.listdir(libraries)
    except OSError:
        return []
    return sorted((n for n in names if VERSION_PATTERN.match(n)), key=parse_version)


def get_houdini_materialx_version():
    """Returns the MaterialX version Houdini ships, e.g. "1.39.5", or None if it can't be found."""
    try:
        import MaterialX
        return MaterialX.getVersionString()
    except ImportError:
        pass
    stdlib = os.path.join(os.environ.get("HFS", ""), "houdini", "materialx", "libraries", "stdlib", "stdlib_defs.mtlx")
    try:
        with open(stdlib, encoding="utf-8") as f:
            header = f.read(1024)
    except OSError:
        return None
    match = re.search(r'<materialx\s+version="([\d.]+)"', header)
    return match.group(1) if match else None


def parse_version(version):
    return tuple(int(part) for part in version.split("."))


def choose_materialx_version(houdini_version, available_versions):
    """Picks the library version closest to Houdini's MaterialX without being newer than it, when possible.

    A newer library could let SLX code use nodes that Houdini's renderers don't know about.
    """
    available = sorted(available_versions, key=parse_version)
    if not available:
        return None
    if not houdini_version:
        return available[-1]
    target = parse_version(houdini_version)
    same_minor = [v for v in available if parse_version(v)[:2] == target[:2]]
    if same_minor:
        if len(target) < 3:
            return same_minor[-1]
        not_newer = [v for v in same_minor if parse_version(v) <= target]
        return not_newer[-1] if not_newer else same_minor[0]
    older = [v for v in available if parse_version(v) < target]
    return older[-1] if older else available[0]


def find_source_files(path):
    """Returns the SLX file and every file it includes, following #include and #library recursively.

    The result can contain files that don't exist yet, so a watcher notices when they appear.
    """
    main_directory = os.path.dirname(os.path.abspath(path))
    found = []
    pending = [os.path.abspath(path)]
    while pending:
        current = os.path.normpath(pending.pop(0))
        if current in found:
            continue
        found.append(current)
        if not current.endswith(".mxsl"):
            continue
        try:
            with open(current, encoding="utf-8") as f:
                text = f.read()
        except OSError:
            continue
        for directive, name in INCLUDE_PATTERN.findall(text):
            # mxslc resolves #library paths relative to the main file, even inside included files
            base = main_directory if directive == "library" else os.path.dirname(current)
            pending.append(os.path.join(base, name))
    return found


def parse_value(text):
    """Converts parameter text to a value mxslc accepts: 0.5, 2, true, "a.png" or (1, 0, 0).

    Text that isn't a literal is passed through as a string, so file names don't need quotes.
    """
    stripped = text.strip()
    if stripped.lower() in ("true", "false"):
        return stripped.lower() == "true"
    try:
        value = ast.literal_eval(stripped)
    except (ValueError, SyntaxError):
        return stripped
    if isinstance(value, tuple):
        return list(value)
    return value


def parse_value_list(text):
    """Converts comma separated parameter text, e.g. `0.5, "a.png", (1, 0, 0)`, to a list of values."""
    parts = []
    depth = 0
    quote = None
    current = ""
    for char in text:
        if quote:
            if char == quote:
                quote = None
        elif char in "\"'":
            quote = char
        elif char in "([{":
            depth += 1
        elif char in ")]}":
            depth -= 1
        elif char == "," and depth == 0:
            parts.append(current)
            current = ""
            continue
        current += char
    if current.strip():
        parts.append(current)
    return [parse_value(part) for part in parts]


def has_sequence_value(values):
    return any(isinstance(v, list) for v in values)


def read_template():
    with open(TEMPLATE_PATH, encoding="utf-8") as f:
        return f.read()
