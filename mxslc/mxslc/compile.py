from pathlib import Path

from .Preprocessor.process import process as preprocess
from .document import get_document
from .file_utils import pkg_path
from .load_library import load_standard_library
from .parse import parse
from .scan import scan


def compile_(source: str | Path, mtlx_version: str | None, include_dirs: list[Path], is_main: bool) -> str:
    tokens = scan(pkg_path(r"libraries/slxlib_defs.mxsl")) + scan(source)
    processed_tokens, mtlx_version = preprocess(tokens, mtlx_version, include_dirs, is_main=is_main)
    statements = parse(processed_tokens)
    get_document().set_version_string(mtlx_version)
    load_standard_library(mtlx_version)
    for statement in statements:
        statement.execute()
    return mtlx_version
