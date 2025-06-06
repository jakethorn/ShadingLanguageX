from pathlib import Path
from typing import Any

from .Keyword import AliasType, Keyword, DataType
from .token_types import FLOAT_LITERAL, INT_LITERAL, STRING_LITERAL, FILENAME_LITERAL


class Token:
    def __init__(self, type_: str, lexeme: str = None, file: Path = None, line: int = None):
        self.__type = type_
        self.__lexeme = lexeme or type_
        self.__file = file
        self.__line = line

        # account for aliases
        if self.__type in AliasType:
            self.__type = AliasType(self.__type).real

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

    def __eq__(self, other: Any) -> bool:
        if isinstance(other, str):
            return self.type == other
        if isinstance(other, Token):
            return self.lexeme == other.lexeme
        return super().__eq__(other)

    def __str__(self) -> str:
        if self.type == ";":
            return self.type + "\n"
        elif self.type == self.lexeme:
            return self.type
        else:
            return f"({self.type}: {self.lexeme})"
