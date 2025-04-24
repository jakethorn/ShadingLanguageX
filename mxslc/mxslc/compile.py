from pathlib import Path

from . import mtlx, state
from .Preprocess import macros
from .Preprocess.process import process as preprocess
from .Statements import Statement
from .parse import parse
from .post_process import post_process
from .scan import scan


def compile_file(mxsl_path: str | Path, mtlx_path: str | Path = None, *, add_include_dirs: list[Path] = None) -> None:
    mxsl_filepaths = _handle_mxsl_path(mxsl_path)

    for mxsl_filepath in mxsl_filepaths:
        mtlx_filepath = _handle_mtlx_path(mtlx_path, mxsl_filepath)

        macros.clear()
        mtlx.clear()
        state.clear()

        include_dirs = (add_include_dirs or []) + [mxsl_filepath.parent, Path(".")]

        tokens = scan(mxsl_filepath)
        processed_tokens = preprocess(tokens, include_dirs)
        statements = parse(processed_tokens)
        _compile(statements)
        post_process()

        with open(mtlx_filepath, "w") as file:
            file.write(mtlx.get_xml())

        print(f"{mxsl_filepath.name} compiled successfully.")


def _handle_mxsl_path(mxsl_path: str | Path) -> list[Path]:
    if mxsl_path is None:
        raise TypeError("Path to .mxsl file was empty.")
    if not isinstance(mxsl_path, str | Path):
        raise TypeError(f"Path to .mxsl file was an invalid type: '{type(mxsl_path)}'.")
    mxsl_path = Path(mxsl_path)
    if not mxsl_path.exists():
        raise FileNotFoundError(f"No such file or directory: '{mxsl_path}'.")
    if mxsl_path.is_file():
        return [mxsl_path]
    if mxsl_path.is_dir():
        return list(mxsl_path.glob("*.mxsl"))
    raise ValueError("Bad mxsl_path.")


def _handle_mtlx_path(mtlx_path: str | Path | None, mxsl_file: Path) -> Path:
    if mtlx_path is None:
        return mxsl_file.with_suffix(".mtlx")
    if not isinstance(mtlx_path, str | Path):
        raise TypeError(f"Path to .mtlx file was an invalid type: '{type(mtlx_path)}'.")
    mtlx_path = Path(mtlx_path)
    if mtlx_path.is_file():
        return mtlx_path
    if mtlx_path.is_dir():
        return mtlx_path / (mxsl_file.stem + ".mtlx")
    if mtlx_path.suffix != ".mtlx":
        mtlx_path /= (mxsl_file.stem + ".mtlx")
    mtlx_path.parent.mkdir(parents=True, exist_ok=True)
    return mtlx_path


def _compile(statements: list[Statement]) -> None:
    for statement in statements:
        statement.execute()
