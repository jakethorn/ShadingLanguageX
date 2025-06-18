from __future__ import annotations
from pathlib import Path

from .Keyword import Keyword
from .token_types import FLOAT_LITERAL, INT_LITERAL, STRING_LITERAL, FILENAME_LITERAL, IDENTIFIER


class Token:
    def __init__(self, type_: str, lexeme: str = None, file: Path = None, line: int = None):
        self.__type = type_
        self.__lexeme = lexeme or type_
        self.__file = file
        self.__line = line

        # data type aliases
        if self.__type == Keyword.BOOL:
            self.__type = Keyword.BOOLEAN
        if self.__type == Keyword.INT:
            self.__type = Keyword.INTEGER
        if self.__type == Keyword.VEC2:
            self.__type = Keyword.VECTOR2
        if self.__type == Keyword.VEC3:
            self.__type = Keyword.VECTOR3
        if self.__type == Keyword.VEC4:
            self.__type = Keyword.VECTOR4

        # parse value
        self.__value = None
        if self.__type == Keyword.TRUE:
            self.__value = True
        if self.__type == Keyword.FALSE:
            self.__value = False
        if self.__type == FLOAT_LITERAL:
            self.__value = float(lexeme)
        if self.__type == INT_LITERAL:
            self.__value = int(lexeme)
        if self.__type == STRING_LITERAL:
            self.__value = lexeme.strip('"')
        if self.__type == FILENAME_LITERAL:
            self.__value = Path(lexeme.strip('"'))

    @property
    def type(self) -> str:
        return self.__type

    @property
    def lexeme(self) -> str:
        return self.__lexeme

    @property
    def value(self) -> bool | int | float | str | Path:
        return self.__value

    @property
    def file(self) -> Path:
        return self.__file

    @property
    def line(self) -> int:
        return self.__line

    def __eq__(self, other: str | Token) -> bool:
        if isinstance(other, str):
            return self.type == other
        if isinstance(other, Token):
            return self.lexeme == other.lexeme
        return False

    def __str__(self) -> str:
        return self.lexeme

    def __hash__(self) -> int:
        return hash(self.type)


class IdentifierToken(Token):
    def __init__(self, lexeme: str, file: Path = None, line: int = None):
        super().__init__(IDENTIFIER, lexeme, file, line)
