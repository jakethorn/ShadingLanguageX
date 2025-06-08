from __future__ import annotations

from . import mx_utils
from .DataType import DataType
from .Expressions import Expression
from .Token import Token, IdentifierToken


class Argument:
    """
    Represents a positional or named argument to a function, constructor or node constructor.
    """
    def __init__(self, expr: Expression, position: int, name: Token = None):
        self.__expr = expr
        self.__position = position
        self.__name = name.lexeme if name is not None else None

    @property
    def position(self) -> int:
        return self.__position

    @property
    def name(self) -> str:
        return self.__name

    @property
    def data_type(self) -> DataType:
        return self.__expr.data_type

    @property
    def is_positional(self) -> bool:
        return self.__name is None

    @property
    def is_named(self) -> bool:
        return self.__name is not None

    @property
    def expression(self) -> Expression:
        return self.__expr

    def instantiate_templated_types(self, data_type: DataType) -> Argument:
        return Argument(self.__expr.instantiate_templated_types(data_type), self.position, IdentifierToken(self.name))

    def init(self, valid_types: DataType | list[DataType] = None) -> None:
        self.__expr.init(valid_types)

    def evaluate(self) -> mx_utils.Node:
        return self.__expr.evaluate()
