from pathlib import Path

from . import state
from .Function import NodeGraphFunction
from .Preprocessor.process import process as preprocess
from .document import get_document
from .file_utils import pkg_path
from .mx_wrapper import Document
from .parse import parse
from .scan import scan


def compile_(source: str | Path, include_dirs: list[Path], is_main: bool) -> None:
    tokens = scan(pkg_path(r"slxlib/slxlib_defs.mxsl")) + scan(source)
    processed_tokens, mtlx_version = preprocess(tokens, include_dirs, is_main=is_main)
    statements = parse(processed_tokens)
    get_document().set_version_string(mtlx_version)
    _load_standard_library(mtlx_version)
    for statement in statements:
        statement.execute()


def _load_standard_library(mtlx_version: str) -> None:
    document = Document()
    document.set_version_string(mtlx_version)
    document.load_standard_library(mtlx_version)
    for nd in document.node_defs:
        if not nd.is_default_version:
            continue
        function = NodeGraphFunction.from_node_def(nd)
        state.add_function(function)
