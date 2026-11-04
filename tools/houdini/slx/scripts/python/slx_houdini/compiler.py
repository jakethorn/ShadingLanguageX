"""Finds and configures the mxslc compiler for the running Houdini.

Nothing in this module imports hou, so it can be tested with a plain Python interpreter.
"""

import functools
import glob
import os
import re
import sys

MODULE_DIRECTORY = os.path.dirname(os.path.abspath(__file__))
PLUGIN_ROOT = os.path.normpath(os.path.join(MODULE_DIRECTORY, "..", "..", ".."))  # slx, above scripts/python/slx_houdini
VENDOR_ROOT = os.path.join(PLUGIN_ROOT, "vendor")
TEMPLATE_PATH = os.path.join(MODULE_DIRECTORY, "templates", "material.mxsl")

INCLUDE_PATTERN = re.compile(r'^[ \t]*#[ \t]*(include|library)[ \t]+"([^"]+)"', re.MULTILINE)
VERSION_PATTERN = re.compile(r"^\d+\.\d+\.\d+$")


def get_vendor_directory(vendor_root=VENDOR_ROOT):
    """Returns the folder holding the mxslc build for this OS and Python version, or None if there isn't one.

    vendor_mxslc.py names the folder <os>-cp<XY>-<hash of the build>, e.g. windows-cp313-1a2b3c4d.
    """
    os_name = {"win32": "windows", "darwin": "macos"}.get(sys.platform, "linux")
    candidates = glob.glob(os.path.join(vendor_root, "{}-cp{}{}-*".format(os_name, *sys.version_info[:2])))
    return max(candidates, key=os.path.getmtime) if candidates else None


def import_mxslc():
    """Imports mxslc, preferring the copy vendored with the plugin over any installed one."""
    vendor_directory = get_vendor_directory()
    if vendor_directory is not None and vendor_directory not in sys.path:
        sys.path.insert(0, vendor_directory)
    try:
        import mxslc
    except ImportError as e:
        raise ImportError(
            "Could not import mxslc ({}). Run tools/houdini/vendor_mxslc.py to download it into {}, "
            "or pip install mxslcxx into Houdini's Python.".format(e, VENDOR_ROOT)
        ) from e
    return mxslc


def get_available_materialx_versions(mxslc):
    """Returns the MaterialX library versions mxslc can compile for, oldest first."""
    try:
        names = os.listdir(os.path.join(os.path.dirname(mxslc.__file__), "libraries"))
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


@functools.lru_cache(maxsize=None)
def choose_materialx_version_for_houdini(mxslc):
    """Returns the library version mxslc compiles for by default in Houdini, or None if it has none."""
    return choose_materialx_version(get_houdini_materialx_version(), get_available_materialx_versions(mxslc))


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


def find_source_files(path, search_directories=()):
    """Returns the SLX file and every file it includes, following #include and #library recursively.

    Like mxslc, a file that isn't found relative to the file naming it is looked for in each search directory.
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
            pending.append(find_included_file(name, [base] + list(search_directories)))
    return found


def find_included_file(name, directories):
    """Returns the path of name in the first directory that has it, or in the first directory if none do."""
    candidates = [os.path.join(directory, name) for directory in directories]
    return next((c for c in candidates if os.path.isfile(c)), candidates[0])


def has_sequence_value(values):
    return any(isinstance(v, list) for v in values)


def read_template():
    with open(TEMPLATE_PATH, encoding="utf-8") as f:
        return f.read()
