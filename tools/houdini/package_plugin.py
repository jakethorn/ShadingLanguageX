"""Builds ShadingLanguageX-Houdini.zip, the ready-to-install plugin that releases ship.

    python package_plugin.py --wheel dist/*.whl        # wheels from a build, as the release workflow does
    python package_plugin.py --version 0.3.3           # wheels of an mxslcxx release on PyPI

The zip holds the plugin folder, with its examples and the compiler for every OS and Python version Houdini uses,
the package file and the README.
"""

import argparse
import glob
import os
import shutil
import sys
import tempfile

import vendor_mxslc

HOUDINI_DIRECTORY = os.path.dirname(os.path.abspath(__file__))
PLUGIN_NAME = "ShadingLanguageX-Houdini"

sys.path.insert(0, os.path.join(HOUDINI_DIRECTORY, "slx", "scripts", "python"))
from slx_houdini import examples  # noqa: E402


def package_plugin(wheels, output_path):
    stage_directory = tempfile.mkdtemp()
    try:
        root = os.path.join(stage_directory, PLUGIN_NAME)
        shutil.copytree(os.path.join(HOUDINI_DIRECTORY, "slx"), os.path.join(root, "slx"),
                        ignore=shutil.ignore_patterns("vendor", "__pycache__"))
        vendor_directory = os.path.join(root, "slx", "vendor")
        vendor_mxslc.vendor_wheels(wheels, vendor_directory)
        check_examples(vendor_directory, os.path.join(root, "slx", "examples"))
        for name in ("README.md", "ShadingLanguageX.json"):
            shutil.copyfile(os.path.join(HOUDINI_DIRECTORY, name), os.path.join(root, name))
        archive = shutil.make_archive(os.path.splitext(output_path)[0], "zip", stage_directory, PLUGIN_NAME)
    finally:
        shutil.rmtree(stage_directory, ignore_errors=True)
    print("{} ({:.1f} MB)".format(archive, os.path.getsize(archive) / 1048576))


def check_examples(vendor_directory, examples_directory):
    """Stops packaging if an example doesn't compile with the vendored compiler, so the plugin never ships a broken one."""
    mxslc = import_vendored_mxslc(vendor_directory)
    if mxslc is None:
        return
    # The SLX Material node's default settings: globals keep their initializers, and functions are inlined.
    settings = {"error_on_missing_globals": False}
    if hasattr(mxslc, "FunctionBehaviour"):
        settings["default_function_behaviour"] = mxslc.FunctionBehaviour.Inline
    failures = []
    for path in examples.get_examples(examples_directory):
        try:
            mxslc.compile_file_to_string(path, mxslc.CompileOptions(**settings))
        except RuntimeError as e:
            failures.append("{}: {}".format(os.path.basename(path), str(e).splitlines()[0]))
    if failures:
        sys.exit("These examples don't compile with the packaged compiler:\n  " + "\n  ".join(failures))


def import_vendored_mxslc(vendor_directory):
    """Imports the vendored mxslc for the Python running this script, or returns None if there isn't one."""
    os_name = {"win32": "windows", "darwin": "macos"}.get(sys.platform, "linux")
    candidates = glob.glob(os.path.join(vendor_directory, "{}-cp{}{}-*".format(os_name, *sys.version_info[:2])))
    if not candidates:
        print("  no compiler for this Python, so the examples aren't checked")
        return None
    sys.path.insert(0, candidates[0])
    import mxslc
    return mxslc


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--version", help="the mxslcxx release to download (default: latest)")
    parser.add_argument("--wheel", nargs="+", help="wheel files to package instead of downloading")
    parser.add_argument("--output", default=PLUGIN_NAME + ".zip", help="the zip file to write")
    args = parser.parse_args()
    if args.wheel:
        wheels = vendor_mxslc.read_local_wheels(args.wheel)
    else:
        wheels = vendor_mxslc.download_release_wheels(args.version)
    package_plugin(wheels, os.path.abspath(args.output))


if __name__ == "__main__":
    main()
