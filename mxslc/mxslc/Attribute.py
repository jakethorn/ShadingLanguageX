from .CompileError import CompileError
from .Token import Token


class Attribute:
    def __init__(self, child: Token | None, name: Token, value: Token):
        self.__child = child
        self.__name = name
        self.__value = value

    @property
    def child(self) -> str | None:
        return self.__child.lexeme if self.__child is not None else None

    @child.setter
    def child(self, value: Token) -> None:
        if self.__child is not None:
            raise CompileError(f"Invalid attribute syntax: @{self.__child}.{self.__name} {self.__value}.", self.__name)
        self.__child = value

    @property
    def name(self) -> str:
        return self.__name.lexeme

    @property
    def value(self) -> str:
        return self.__value.value
