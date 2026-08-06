from importlib import import_module
from importlib.metadata import PackageNotFoundError, version as dist_version

# Load the compiled extension module.
_native = import_module("._mxslc", package=__name__)

__doc__ = getattr(_native, "__doc__", None)
__version__ = getattr(_native, "__version__", None)

# Re-export public native symbols at package level.
for _name in dir(_native):
    if not _name.startswith("_"):
        globals()[_name] = getattr(_native, _name)

# Fall back to distribution metadata when native version is unavailable.
if __version__ is None:
    try:
        __version__ = dist_version("mxslcxx")
    except PackageNotFoundError:
        # Source-tree fallback when package metadata is not installed.
        __version__ = "0+unknown"
