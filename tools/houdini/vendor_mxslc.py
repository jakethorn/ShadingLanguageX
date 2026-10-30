"""Puts the mxslc Python bindings into slx/vendor, one folder per OS and Python version.

Houdini versions use different Python versions, so the plugin carries a build for each, and loads the one
matching the running Houdini (see slx_houdini.compiler.get_vendor_directory).

    python vendor_mxslc.py                      # every wheel of the latest mxslcxx release on PyPI
    python vendor_mxslc.py --version 0.3.2      # a specific release
    python vendor_mxslc.py --wheel dist/*.whl   # local wheels, e.g. a development build
"""

import argparse
import io
import json
import os
import re
import shutil
import urllib.request
import zipfile

VENDOR_DIRECTORY = os.path.join(os.path.dirname(os.path.abspath(__file__)), "slx", "vendor")
WHEEL_PATTERN = re.compile(r"^mxslcxx-[^-]+-cp(\d+)-cp\d+-(.+)\.whl$")


def get_vendor_name(wheel_name):
    """Returns the vendor folder for a wheel, e.g. "windows-cp311", or None for wheels Houdini can't use."""
    match = WHEEL_PATTERN.match(wheel_name)
    if not match:
        return None
    python_tag, platform_tag = match.groups()
    if platform_tag.startswith("win_amd64"):
        os_name = "windows"
    elif "manylinux" in platform_tag and platform_tag.endswith("x86_64"):
        os_name = "linux"
    elif platform_tag.startswith("macosx"):
        os_name = "macos"
    else:
        return None
    return "{}-cp{}".format(os_name, python_tag)


def download_release_wheels(version):
    url = "https://pypi.org/pypi/mxslcxx/{}/json".format(version) if version else "https://pypi.org/pypi/mxslcxx/json"
    with urllib.request.urlopen(url) as response:
        release = json.load(response)
    print("mxslcxx {}".format(release["info"]["version"]))
    for file in release["urls"]:
        if file["packagetype"] == "bdist_wheel" and get_vendor_name(file["filename"]):
            with urllib.request.urlopen(file["url"]) as response:
                yield file["filename"], response.read()


def read_local_wheels(paths):
    for path in paths:
        with open(path, "rb") as f:
            yield os.path.basename(path), f.read()


def extract_wheel(wheel_name, data):
    target = os.path.join(VENDOR_DIRECTORY, get_vendor_name(wheel_name))
    shutil.rmtree(target, ignore_errors=True)
    with zipfile.ZipFile(io.BytesIO(data)) as wheel:
        for member in wheel.namelist():
            if ".dist-info/" not in member:
                wheel.extract(member, target)
    print("  {} -> {}".format(wheel_name, os.path.relpath(target)))


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--version", help="the mxslcxx release to download (default: latest)")
    parser.add_argument("--wheel", nargs="+", help="local wheel files to use instead of downloading")
    args = parser.parse_args()

    wheels = read_local_wheels(args.wheel) if args.wheel else download_release_wheels(args.version)
    for wheel_name, data in wheels:
        if get_vendor_name(wheel_name):
            extract_wheel(wheel_name, data)
        else:
            print("  skipped {}: not a platform Houdini runs on".format(wheel_name))


if __name__ == "__main__":
    main()
