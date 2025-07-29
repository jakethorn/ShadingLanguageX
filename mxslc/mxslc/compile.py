from pathlib import Path

from . import state
from .Function import NodeGraphFunction
from .Preprocessor.process import process as preprocess
from .file_utils import pkg_path
from .mx_wrapper import Document
from .parse import parse
from .scan import scan


def compile_(source: str | Path, include_dirs: list[Path], is_main: bool) -> None:
        tokens = scan(pkg_path(r"slxlib/slxlib_defs.mxsl")) + scan(source)
        processed_tokens = preprocess(tokens, include_dirs, is_main=is_main)
        statements = parse(processed_tokens)
        _load_standard_library()
        for statement in statements:
            statement.execute()


def _load_standard_library() -> None:
    document = Document()
    document.load_standard_library()
    for nd in document.node_defs:
        if not nd.is_default_version:
            continue
        function = NodeGraphFunction.from_node_def(nd)
        state.add_function(function)
