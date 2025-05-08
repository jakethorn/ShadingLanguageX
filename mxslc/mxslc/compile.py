from pathlib import Path

from .Preprocess.process import process as preprocess
from .parse import parse
from .scan import scan


def compile_(source: str | Path, include_dirs: list[Path], is_main: bool) -> None:
        tokens = scan(source)
        processed_tokens = preprocess(tokens, include_dirs, is_main=is_main)
        statements = parse(processed_tokens)
        for statement in statements:
            statement.execute()
