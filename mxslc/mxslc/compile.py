from pathlib import Path

from . import state
from .Function import Function
from .Preprocessor.process import process as preprocess
from .mx_wrapper import Document
from .parse import parse
from .scan import scan


def compile_(source: str | Path, include_dirs: list[Path], is_main: bool) -> None:
        tokens = scan(source)
        processed_tokens = preprocess(tokens, include_dirs, is_main=is_main)
        statements = parse(processed_tokens)
        _load_standard_library()
        for statement in statements:
            statement.execute()


def _load_standard_library() -> None:
    document = Document()
    document.load_standard_library()
    for nd in document.node_defs:
        # TODO add support for multiple return values
        if nd.output_count > 1:
            continue
        if not nd.is_default_version:
            continue
        function = Function.from_node_def(nd)
        state.add_function(function)
