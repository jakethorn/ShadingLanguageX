from pathlib import Path

from .Directive import DIRECTIVES, DEFINE, UNDEF, IF, IFDEF, IFNDEF, INCLUDE, PRAGMA, PRINT, ELIF, ELSE, ENDIF
from .macros import define as define_macro, undefine as undefine_macro, is_defined as is_defined_macro, replace as replace_macro
from .parse import parse
from ..CompileError import CompileError
from ..Token import Token
from ..TokenReader import TokenReader
from ..scan import scan
from ..token_types import IDENTIFIER, EOL


# TODO
# c/c++ macros are deferred, they are not expanded until used
# however i expand during definition
# this can lead to differences in behaviour


def process(tokens: list[Token], include_dirs: list[Path], is_main: bool) -> list[Token]:
    return Processor(tokens, include_dirs, is_main).process()


class Processor(TokenReader):
    def __init__(self, tokens: list[Token], include_dirs: list[Path], is_main: bool):
        super().__init__(tokens)
        self.__include_dirs = include_dirs
        self.__is_main = is_main

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
        raise AssertionError()

    def __process_define(self) -> list[Token]:
        self._match(DEFINE)
        identifier = self._match(IDENTIFIER)
        value = []
        while self._peek() != EOL:
            value.extend(self.__process_next())
        self._match(EOL)
        define_macro(identifier, value)
        return []

    def __process_undef(self) -> list[Token]:
        self._match(UNDEF)
        identifier = self._match(IDENTIFIER)
        self._match(EOL)
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
            condition_tokens = []
            while self._peek() != EOL:
                condition_tokens.extend(self.__process_next())
            condition_tokens.append(self._match(EOL))
            condition = parse(condition_tokens)
        elif branch_type == IFDEF:
            condition = is_defined_macro(self._match(IDENTIFIER))
            self._match(EOL)
        elif branch_type == IFNDEF:
            condition = not is_defined_macro(self._match(IDENTIFIER))
            self._match(EOL)
        else:
            condition = True
            self._match(EOL)
        tokens = []
        while self._peek() not in [ELIF, ELSE, ENDIF]:
            tokens.extend(self.__process_next())
        return condition, tokens

    def __process_include(self) -> list[Token]:
        self._match(INCLUDE)
        path_tokens = []
        while self._peek() != EOL:
            path_tokens.extend(self.__process_next())
        path_tokens.append(self._match(EOL))
        path = parse(path_tokens)
        included_files = self.__search_in_include_dirs(path)
        included_tokens = []
        for included_file in included_files:
            included_tokens.extend(process(scan(included_file), self.__new_include_dirs(included_file.parent), is_main=False))
        self.__define_main()
        return included_tokens

    def __process_pragma(self) -> list[Token]:
        # TODO implement pragma directives
        return []

    def __process_print(self) -> list[Token]:
        # TODO implement print directive
        return []

    def __process_non_directive(self) -> list[Token]:
        token = self._consume()
        if token == EOL:
            return []
        if is_defined_macro(token):
            return replace_macro(token)
        return [token]

    def __define_main(self) -> None:
        if self.__is_main:
            define_macro("__MAIN__")
            undefine_macro("__INCLUDE__")
        else:
            define_macro("__INCLUDE__")
            undefine_macro("__MAIN__")

    def __search_in_include_dirs(self, path: str) -> list[Path]:
        if not isinstance(path, str):
            raise CompileError(-1, "Path must be a string.")

        path = Path(path)
        if path.is_absolute():
            if path.is_file():
                return [path]
            if path.is_dir():
                return list(path.glob("*.mxsl"))

        for include_dir in self.__include_dirs:
            full_path = include_dir / path
            if full_path.is_file():
                return [full_path]
            if full_path.is_dir():
                return list(full_path.glob("*.mxsl"))

        # TODO fix -1
        raise CompileError(-1, f"File or directory not found: {path}.")

    def __new_include_dirs(self, local_dir: Path) -> list[Path]:
        copy = self.__include_dirs[:]
        copy[-2] = local_dir
        return copy
