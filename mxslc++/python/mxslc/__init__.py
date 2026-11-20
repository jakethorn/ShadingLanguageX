from importlib.metadata import PackageNotFoundError, version as dist_version

# Re-export the compiled extension module's public names. A plain star import,
# unlike a dynamic loop, lets type checkers and IDEs see them through the
# _mxslc.pyi stubs.
from . import _mxslc
from ._mxslc import *

__doc__ = _mxslc.__doc__
__version__ = getattr(_mxslc, "__version__", None)

# Fall back to distribution metadata when native version is unavailable.
if __version__ is None:
    try:
        __version__ = dist_version("mxslcxx")
    except PackageNotFoundError:
        # Source-tree fallback when package metadata is not installed.
        __version__ = "0+unknown"
