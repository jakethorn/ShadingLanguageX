from pathlib import Path
from typing import Sequence

from . import mx_utils, state
from .Interactive.ShaderInterface import ShaderInterface
from .Preprocessor.macros import undefine_all_macros, Macro, define_macro
from .compile import compile_
from .file_utils import handle_input_path, handle_output_path
from .post_process import post_process


def compile_file(mxsl_path: str | Path,
                 mtlx_path: str | Path = None,
                 *,
                 main_func: str = None,
                 main_args: Sequence[mx_utils.Value] = None,
                 add_include_dirs: Sequence[Path] = None,
                 add_macros: Sequence[str | Macro] = None) -> None:
    main_args = main_args or []
    add_include_dirs = add_include_dirs or []
    add_macros = add_macros or []

    mxsl_filepaths = handle_input_path(mxsl_path)

    for mxsl_filepath in mxsl_filepaths:
        mtlx_filepath = handle_output_path(mtlx_path, mxsl_filepath)

        undefine_all_macros()
        mx_utils.clear()
        state.clear()

        include_dirs = add_include_dirs + [mxsl_filepath.parent, Path(".")]

        for macro in add_macros:
            define_macro(macro)

        compile_(mxsl_filepath, include_dirs, is_main=True)
        _call_main(mxsl_filepath, main_func, main_args)
        post_process()

        with open(mtlx_filepath, "w") as file:
            file.write(mx_utils.get_xml())

        print(f"{mxsl_filepath.name} compiled successfully.")


def _call_main(file: Path, name: str | None, args: Sequence[mx_utils.Value]) -> None:
    shader = ShaderInterface()
    if name is None:
        if "main" in shader and shader.main.file == file:
            shader.main(*args)
    else:
        shader[name](*args)
