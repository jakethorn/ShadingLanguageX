from pathlib import Path
from typing import Sequence

import MaterialX as mx

from .ShaderInterface import ShaderInterface
from .. import state
from ..Preprocessor.macros import undefine_all_macros
from ..compile import compile_
from ..document import get_document, new_document
from ..file_utils import handle_input_path
from ..post_process import post_process


class InteractiveCompiler:
    def __init__(self, add_include_dirs: Sequence[Path] = None):
        self.__add_include_dirs = add_include_dirs or []
        self.clear()

    @property
    def document(self) -> mx.Document:
        return get_document()

    @property
    def xml(self) -> str:
        return self.document.xml

    def get_shader_interface(self) -> ShaderInterface:
        return ShaderInterface()

    def include(self, mxsl_path: str | Path) -> None:
        mxsl_filepaths = handle_input_path(mxsl_path)

        for mxsl_filepath in mxsl_filepaths:
            include_dirs = [*self.__add_include_dirs, mxsl_filepath.parent, Path(".")]
            compile_(mxsl_filepath, include_dirs, is_main=False)

    def eval(self, code_snippet: str) -> None:
        include_dirs = [*self.__add_include_dirs, Path(".")]
        compile_(code_snippet, include_dirs, is_main=True)

    def save(self, mtlx_filepath: Path, mkdir=False) -> None:
        post_process()
        mtlx_filepath = mtlx_filepath.resolve()
        if mkdir:
            mtlx_filepath.parent.mkdir(parents=True, exist_ok=True)
        elif not mtlx_filepath.parent.is_dir():
            raise FileNotFoundError(f"No such directory: '{mtlx_filepath.parent}'. Set mkdir=True to create it when saving the file.")
        with open(mtlx_filepath, "w") as file:
            file.write(self.xml)

    def clear(self) -> None:
        undefine_all_macros()
        new_document()
        state.clear()
