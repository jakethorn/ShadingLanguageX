from pathlib import Path
from typing import Sequence

import MaterialX as mx

from . import mtlx, state
from .Interactive import globals_
from .Preprocess import macros
from .compile import compile_
from .file_utils import handle_mxsl_path, handle_mtlx_path
from .post_process import post_process


def compile_file(mxsl_path: str | Path,
                 mtlx_path: str | Path = None,
                 *,
                 main_function: str = None,
                 main_args: Sequence[mtlx.Constant] = None,
                 add_include_dirs: Sequence[Path] = None) -> None:
    mxsl_filepaths = handle_mxsl_path(mxsl_path)

    for mxsl_filepath in mxsl_filepaths:
        mtlx_filepath = handle_mtlx_path(mtlx_path, mxsl_filepath)

        macros.clear()
        mtlx.clear()
        state.clear()

        include_dirs = (add_include_dirs or []) + [mxsl_filepath.parent, Path(".")]

        compile_(mxsl_filepath, include_dirs, is_main=True)
        _call_main(mxsl_filepath, main_function, main_args or [])
        post_process()

        with open(mtlx_filepath, "w") as file:
            file.write(mtlx.get_xml())

        print(f"{mxsl_filepath.name} compiled successfully.")


def _call_main(file: Path, name: str | None, args: Sequence[mx.Node | mtlx.Constant]) -> None:
    if name is None:
        if "main" in globals_ and globals_.main.file == file:
            globals_.main(args)
    else:
        globals_[name](args)
