from pathlib import Path

from . import state
from .Function import NodeGraphFunction
from .mx_wrapper import Document


def load_library(library: str | Path | Document, nodedef_filter: list[str] = None) -> None:
    if isinstance(library, str | Path):
        library = Document(library)
    for nd in library.node_defs:
        if nodedef_filter is not None and nd.node_string not in nodedef_filter:
            continue
        if not nd.is_default_version:
            continue
        function = NodeGraphFunction.from_node_def(nd)
        state.add_function(function)


def load_standard_library(mtlx_version: str | None) -> None:
    library = Document()
    library.load_standard_library(mtlx_version)
    load_library(library)
