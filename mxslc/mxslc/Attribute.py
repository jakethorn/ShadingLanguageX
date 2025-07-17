from .Token import Token


class Attribute:
    def __init__(self, child: Token | None, name: Token, value: Token):
        self.__child = child
        self.__name = name
        self.__value = value

    @property
    def child(self) -> str | None:
        return self.__child.lexeme if self.__child is not None else None

    @property
    def name(self) -> str:
        return self.__name.lexeme

    @property
    def value(self) -> str:
        return self.__value.value
