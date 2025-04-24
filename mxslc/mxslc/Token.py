from pathlib import Path
from typing import Any

from .Keyword import AliasType, Keyword
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
        self.value = None
        if self.__type == Keyword.TRUE:
            self.value = True
        if self.__type == Keyword.FALSE:
            self.value = False
        if self.__type == FLOAT_LITERAL:
            self.value = float(lexeme)
        if self.__type == INT_LITERAL:
            self.value = int(lexeme)
        if self.__type == STRING_LITERAL:
            self.value = lexeme.strip('"')
        if self.__type == FILENAME_LITERAL:
            self.value = Path(lexeme.strip('"'))

    @property
    def type(self) -> str:
        return self.__type

    @property
    def lexeme(self) -> str:
        return self.__lexeme

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
        if self.__type == ";":
            return self.__type + "\n"
        elif self.__type == self.__lexeme:
            return self.__type
        else:
            return f"({self.__type}: {self.__lexeme})"
