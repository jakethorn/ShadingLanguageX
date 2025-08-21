from pathlib import Path
from typing import Type

from .Directive import DIRECTIVES, DEFINE, UNDEF, IF, IFDEF, IFNDEF, INCLUDE, PRAGMA, PRINT, ELIF, ELSE, ENDIF, VERSION, \
    LOADLIB
from .Expression import Primitive
from .macros import Macro, define_macro, undefine_macro, is_macro_defined, run_macro
from .parse import parse as preparse
from .. import state
from ..CompileError import CompileError
from ..Function import NodeGraphFunction
from ..Token import Token
from ..TokenReader import TokenReader
from ..document import end_temporary_document, use_temporary_document, get_document
from ..file_utils import pkg_path
from ..load_library import load_library, load_standard_library
from ..mx_wrapper import Document
from ..parse import parse
from ..post_process import post_process
from ..scan import scan
from ..state import use_temporary_state, end_temporary_state
from ..token_types import IDENTIFIER, EOL


def process(tokens: list[Token], mtlx_version: str | None, include_dirs: list[Path], is_main: bool) -> tuple[list[Token], str]:
    p = Processor(tokens, mtlx_version, include_dirs, is_main)
    return p.process(), p.mtlx_version


class Processor(TokenReader):
    def __init__(self, tokens: list[Token], mtlx_version: str | None, include_dirs: list[Path], is_main: bool):
        super().__init__(tokens)
        self.__include_dirs = include_dirs
        self.__is_main = is_main
        self.__mtlx_version = mtlx_version

    @property
    def mtlx_version(self) -> str:
        return self.__mtlx_version

    def process(self) -> list[Token]:
        processed_tokens = []
        self.__define_main()
        while self._reading_tokens():
            processed_tokens.extend(self.__process_next())
        return processed_tokens

    def __process_next(self) -> list[Token]:
        if self._peek() in DIRECTIVES:
            return self.__process_directive()
        return self.__process_non_directive()

    def __process_until(self, *stop_token: str) -> list[Token]:
        tokens: list[Token] = []
        while self._peek() not in stop_token:
            tokens.extend(self.__process_next())
        return tokens

    def __parse_until(self, *stop_token: str, expected_type: Type[Primitive] = None) -> Primitive:
        tokens = self.__process_until(*stop_token)
        value = preparse(tokens)
        if expected_type is not None and not isinstance(value, expected_type):
            raise CompileError(f"Incorrect data type. Expected a {expected_type.__name__}, but got a {type(value).__name__}.", tokens[-1])
        return value

    def __process_directive(self) -> list[Token]:
        directive = self._peek()
        if directive == DEFINE:
            return self.__process_define()
        if directive == UNDEF:
            return self.__process_undef()
        if directive in [IF, IFDEF, IFNDEF]:
            return self.__process_if()
        if directive == INCLUDE:
            return self.__process_include()
        if directive == PRAGMA:
            return self.__process_pragma()
        if directive == PRINT:
            return self.__process_print()
        if directive == VERSION:
            return self.__process_version()
        if directive == LOADLIB:
            return self.__process_loadlib()
        raise AssertionError

    def __process_non_directive(self) -> list[Token]:
        token = self._consume()
        if token == EOL:
            return []
        if is_macro_defined(token):
            return run_macro(token)
        return [token]

    def __process_define(self) -> list[Token]:
        self._match(DEFINE)
        identifier = self._match(IDENTIFIER)
        value = self.__process_until(EOL)
        define_macro(Macro(identifier, value))
        return []

    def __process_undef(self) -> list[Token]:
        self._match(UNDEF)
        identifier = self._match(IDENTIFIER)
        undefine_macro(identifier)
        return []

    def __process_if(self) -> list[Token]:
        branches = [self.__process_branch()]
        while self._peek() in [ELIF, ELSE]:
            branches.append(self.__process_branch())
        self._match(ENDIF)
        for condition, tokens in branches:
            if condition:
                return tokens
        return []

    def __process_branch(self) -> tuple[bool, list[Token]]:
        branch_type = self._match(IF, IFDEF, IFNDEF, ELIF, ELSE)
        if branch_type in [IF, ELIF]:
            condition = self.__parse_until(EOL)
        elif branch_type == IFDEF:
            condition = is_macro_defined(self._match(IDENTIFIER))
        elif branch_type == IFNDEF:
            condition = not is_macro_defined(self._match(IDENTIFIER))
        else:
            condition = True
        body_tokens = self.__process_until(ELIF, ELSE, ENDIF)
        return condition, body_tokens

    def __process_include(self) -> list[Token]:
        directive = self._match(INCLUDE)
        path = self.__parse_until(EOL, expected_type=str)
        included_files = self.__search_in_include_dirs(directive, path)
        included_tokens = []
        for included_file in included_files:
            if included_file.suffix == ".mtlx":
                self.__include_mtlx_file(included_file)
            else: # included_file.suffix == ".mxsl":
                processed_tokens, _ = process(scan(included_file), self.__mtlx_version, self.__new_include_dirs(included_file.parent), is_main=False)
                included_tokens.extend(processed_tokens)
        self.__define_main()
        return included_tokens

    def __include_mtlx_file(self, mtlx_filepath: Path) -> None:
        document = get_document()
        document.read_from_xml_file(mtlx_filepath)
        temp_doc = Document(mtlx_filepath)
        for temp_node in temp_doc.get_nodes():
            node = document.get_node(temp_node.name)
            state.add_node(temp_node.name, node)
        for temp_node_def in temp_doc.node_defs:
            node_def = document.get_node_def(temp_node_def.name)
            function = NodeGraphFunction.from_node_def(node_def)
            state.add_function(function)

    def __process_pragma(self) -> list[Token]:
        # TODO implement pragma directives
        return []

    def __process_print(self) -> list[Token]:
        # TODO implement print directive
        return []

    def __process_version(self) -> list[Token]:
        self._match(VERSION)
        version_tokens = self.__process_until(EOL)
        self.__mtlx_version = self.__mtlx_version or f"{''.join(str(t) for t in version_tokens)}"
        return []

    def __process_loadlib(self) -> list[Token]:
        directive = self._match(LOADLIB)
        path = self.__parse_until(EOL, "(", expected_type=str)
        nodedef_tokens: list[str] | None = None
        if self._consume("("):
            nodedef_tokens = []
            if not self._consume(")"):
                nodedef_tokens.append(self._match(IDENTIFIER).lexeme)
                while self._consume(","):
                    nodedef_tokens.append(self._match(IDENTIFIER).lexeme)
                self._match(")")
        self.__load_library(directive, path, nodedef_tokens)
        return []

    def __load_library(self, directive: Token, lib_path: str, nodedef_filter: list[str]) -> None:
        lib_filepaths = self.__search_in_include_dirs(directive, lib_path)
        for lib_filepath in lib_filepaths:
            if lib_filepath.suffix == ".mxsl":
                library = self.__compile_library(lib_filepath)
                load_library(library, nodedef_filter)
            else: # lib_filepath.suffix == ".mtlx":
                load_library(lib_filepath, nodedef_filter)

    def __compile_library(self, lib_filepath: Path) -> Document:
        prev_doc = use_temporary_document()
        prev_state = use_temporary_state()

        tokens = scan(pkg_path(r"libraries/slxlib_defs.mxsl")) + scan(lib_filepath)
        processed_tokens, _ = process(tokens, self.__mtlx_version, self.__new_include_dirs(lib_filepath.parent), is_main=False)
        statements = parse(processed_tokens)
        load_standard_library(self.__mtlx_version)
        for statement in statements:
            statement.execute()
        post_process()

        temp_doc = end_temporary_document(prev_doc)
        end_temporary_state(prev_state)
        return temp_doc

    def __define_main(self) -> None:
        if self.__is_main:
            define_macro("__MAIN__")
        else:
            undefine_macro("__MAIN__")

    def __search_in_include_dirs(self, token: Token, path: str | Path) -> list[Path]:
        path = Path(path)
        if path.is_absolute():
            if path.is_file():
                return [path]
            if path.is_dir():
                return [p for p in path.glob("*") if p.suffix in [".mtlx", ".mxsl"]]

        for include_dir in self.__include_dirs:
            full_path = include_dir / path
            if full_path.exists():
                return self.__search_in_include_dirs(token, full_path)

        raise CompileError(f"File or directory not found: {path}.", token)

    def __new_include_dirs(self, local_dir: Path) -> list[Path]:
        copy = self.__include_dirs[:]
        copy[-2] = local_dir
        return copy
